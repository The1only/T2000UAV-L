// mytcpsocket.cpp
// -----------------------------------------------------------------------------
// Platform-independent interface to:
//   - Transponder (serial, USB)
//   - Radar (serial, USB or simulated)
//   - IMU (INS sensor, Bluetooth or serial)
//   - X-Plane via MQTT (for simulation inputs)
//
// Uses Qt 6.10 and some platform-specific code paths for Android / macOS / iOS.
// -----------------------------------------------------------------------------

#define UDP     // TODO: Clarify usage or remove if unused

#include <QTime>
#include <QTimer>
#include <QThread>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
#endif

#ifdef Q_OS_IOS
//   #include "IOS_swift/WitSDK/Sensor/Modular/Processor/Roles/BWT901BLE5_0DataProcessor.swift"
#undef Q_OS_MAC
#endif

#ifdef Q_OS_MAC
#include <QSerialPort>
#endif

#include <QList>
#include <QCoreApplication>
#ifndef Q_OS_IOS
#include <QSerialPortInfo>
#endif
#include <QDebug>

#include "mytcpsocket.h"
#include "wit_c_sdk.h"

#include <QVector>
#include <QString>
#include <QMap>
#include <unistd.h>   // usleep
#include "REG.h"

// Enable internal radar simulator (used when real radar is not present)
#undef SIMULATE_RADAR
typedef struct { double L, F, D; } LFD;

// External C interfaces (implemented elsewhere)
extern bool INS_driver(void *, ComQt *serPorts, ComBt *serPortb, void *func);
extern bool AutoScanSensor();
extern void AutoSetBaud(int);

// ============================================================================
// Constructor / Destructor
// ============================================================================

/**
 * @brief Construct a MyTcpSocket object.
 *
 * @param parent   QObject parent (Qt ownership).
 * @param s        Pointer to log / debug text output widget.
 * @param retx     Callback invoked by serial ports (transponder) when data arrives.
 * @param rety     Callback used to inform about IMU connection status.
 */
MyTcpSocket::MyTcpSocket(QObject *parent,
                         QPlainTextEdit *s,
                         void (*retx)(void *, const char *, uint32_t),
                         void (*rety)(void *, bool use_imu))
    : QObject(parent)
{
    this->ret      = retx;  // C callback for transponder data
    this->ret_imu  = rety;  // C callback to notify IMU status
    this->text     = s;
    this->parent   = parent;

    // ---------------------------------------------------------------------
    // Serial / Bluetooth COM objects
    // ---------------------------------------------------------------------

    // Transponder serial port
    TransponderSerPort = new ComQt(parent);
    TransponderSerPort->setParent(this);
    TransponderSerPort->setRxCallback(ret);             // register C callback

    // Radar serial port
    RadarSerPort = new ComQt(this);
    RadarSerPort->setParent(this);
    RadarSerPort->setRxCallback(doRadar);               // static callback -> MyTcpSocket::doRadar

    // INS / IMU serial port
    INSSerPort = new ComQt(parent);
    INSSerPort->setParent(this);
    INSSerPort->setRxCallback(WitSerialDataIn);         // callback from WIT C SDK

    // Bluetooth IMU port
    bluetootPort = new ComBt(this);
    bluetootPort->setParent(this);
    bluetootPort->setRxCallback(WitSerialDataIn);       // callback from WIT C SDK

    // ---------------------------------------------------------------------
    // Deferred startup using a timer (ensures constructor returns first)
    // ---------------------------------------------------------------------
    timerStart = new QTimer(this);
    timerStart->setSingleShot(false);
    connect(timerStart, SIGNAL(timeout()), this, SLOT(doStart()));
    timerStart->start(200);                             // first step after 200 ms

#ifdef Q_OS_MAC
    // Build a serial-number -> port map on macOS
    map = serialToPortMap();
    qDebug() << map;
#endif

    // ---------------------------------------------------------------------
    // MQTT setup (X-Plane / simulator input)
    // ---------------------------------------------------------------------
    // NOTE: Currently configured for localhost mosquitto / similar broker.
    SERVER_ADDRESS = std::string("tcp://localhost:1883");
    CLIENT_ID      = std::string("transponder");

    mqtt = new MqttClient(SERVER_ADDRESS, CLIENT_ID);
    mqtt->setMessageHandler([this](const std::string &topic, const std::string &payload) {
        try {
            float value = std::stof(payload);
            this->handleUpdate(topic, value);
        } catch (const std::exception &e) {
            qWarning() << "Invalid float in payload:"
                       << QString::fromStdString(payload)
                       << "Error:" << e.what();
        }
    });

    mqtt->connect();
    mqtt->subscribe("xplane/#");
    try {
        mqtt->sendMessage("xplane/topic", "1.0 eMove GUI Controller!");
    } catch (const mqtt::exception &e) {
        printf("MQTT publish error: %s\n", e.what());
    }
}

/**
 * @brief Destructor. Closes all serial ports.
 */
MyTcpSocket::~MyTcpSocket()
{
    TransponderSerPort->close();
    RadarSerPort->close();
    INSSerPort->close();

    qDebug() << "Stopped socket...";
}

// ============================================================================
// Configuration
// ============================================================================

/**
 * @brief Configure which serial numbers / IDs are used.
 *
 * @param imu          IMU/INS device serial number or identifier.
 * @param transponder  Transponder device serial number or identifier.
 * @param radar        Radar device serial number or identifier.
 */
void MyTcpSocket::setSerialPorts(QString imu, QString transponder, QString radar)
{
    _transponder_id = transponder;
    _radar_id       = radar;
    _IMU_id         = imu;
}

// ============================================================================
// macOS specific helpers (serial port enumeration & mapping)
// ============================================================================

#ifdef Q_OS_MAC

/**
 * @brief Enumerate serial ports with extended metadata.
 *
 * @return QVector of PortEntry (one per detected port).
 */
QVector<PortEntry> MyTcpSocket::listSerialPortsDetailed()
{
    QVector<PortEntry> out;
    const auto ports = QSerialPortInfo::availablePorts();
    out.reserve(ports.size());

    for (const QSerialPortInfo &p : ports) {
        PortEntry e;
        e.serial         = p.serialNumber();   // may be empty on some adapters/OSes
        e.portName       = p.portName();
        e.systemLocation = p.systemLocation();
        e.description    = p.description();
        e.manufacturer   = p.manufacturer();
        e.vendorId       = p.hasVendorIdentifier()  ? p.vendorIdentifier()  : 0;
        e.productId      = p.hasProductIdentifier() ? p.productIdentifier() : 0;
        out.push_back(e);
    }
    return out;
}

/**
 * @brief Build a map from serial number -> device path / port name.
 *
 * @param useSystemLocation If true, map to systemLocation; otherwise to portName.
 * @return QMap<serialNumber, portPath>
 */
QMap<QString, QString> MyTcpSocket::serialToPortMap(bool useSystemLocation)
{
    static QString portNum;  // reused temporary

    QMap<QString, QString> result;
    static const auto &x= listSerialPortsDetailed();
    for (const auto &e : listSerialPortsDetailed()) {
        // If FTDI og Profillic...
        if (!e.portName.isEmpty() && (!e.serial.isEmpty() || e.vendorId > 100)) {

            if(e.serial.isEmpty())
                portNum =  QString::number(e.productId);
            else
                portNum = e.serial;

            result.insert(portNum, useSystemLocation ? e.systemLocation : e.portName);

            qDebug() << "SerialPort:" << e.systemLocation;

            // Probe port: open + close to check availability
            QSerialPort *serial_ = new QSerialPort();
            serial_->setPortName(e.systemLocation);

            try {
                if (!serial_->open(QIODevice::ReadWrite)) {
                    qWarning() << "Failed to open" << e.systemLocation << ":"
                               << serial_->errorString();
                }

                if (serial_->isOpen()) {
                    try {
                        serial_->close();
                    } catch (const std::exception &ex) {
                        qWarning() << "Error while closing serial port:"
                                   << "Error:" << ex.what();
                    }
                }
            } catch (const std::exception &ex) {
                qWarning() << "Cannot open serial port:"
                           << "Error:" << ex.what();
            }
            delete serial_;
        }
    }
    qDebug() << result;
    return result;
}

/**
 * @brief Find the serial port path associated with a given device serial number.
 *
 * @param targetSerial Target serial number.
 * @return System location / port name, or empty string if not found.
 */
QString MyTcpSocket::findPort(QString targetSerial)
{
    if (map.contains(targetSerial)) {
        return map.value(targetSerial);
    } else {
        qWarning() << "Device with serial" << targetSerial << "not found";
    }
    return "";
}
#endif  // Q_OS_MAC

// ============================================================================
// Startup state machine (runs on timerStart)
// ============================================================================

/**
 * @brief Progressive startup handler.
 *
 * State machine driven by timerStart:
 *   state 0: Try connect IMU (BT / serial).
 *   state 1: Try connect transponder (USB).
 *   state 2: Keep monitoring / retry transponder.
 * Also optionally generates radar/IMU simulation data when SIMULATE_RADAR is set.
 */
void MyTcpSocket::doStart()
{
    static int state = 0;

    if (state == 0) {
        // Step 0: IMU init
        timerStart->stop();
        connectedIMU();          // try to find & initialize IMU / INS
        timerStart->start(2000);
        ++state;
    } else if (state == 1) {
        // Step 1: Transponder init
        timerStart->stop();

        NoButtonMessageBox *m_msgBoxTrans =
            new NoButtonMessageBox(tr("Looking for USB Transponder!"));
        m_msgBoxTrans->show();
        QCoreApplication::processEvents();

        connected();             // try to find & initialize transponder

        QCoreApplication::processEvents();
        QThread::msleep(2000);

        m_msgBoxTrans->hide();
        delete m_msgBoxTrans;

        // Show second status message depending on detection result
        if (!Transponderstat) {
            m_msgBoxTrans =
                new NoButtonMessageBox(tr("Transponder not found!"));
        } else {
            m_msgBoxTrans =
                new NoButtonMessageBox(tr("Transponder found and connected..."));
        }
        m_msgBoxTrans->show();
        QCoreApplication::processEvents();
        QThread::msleep(2000);
        m_msgBoxTrans->hide();
        delete m_msgBoxTrans;

        // Notify IMU connection state through callback
        this->ret_imu(this->parent, IMUconnected);

        timerStart->start(5000);
        ++state;
    } else if (state == 2) {
        // Step 2: Keep retrying transponder if not connected
        if (!Transponderstat) {
            connected();
        }
        // Keep timer running, used for backlight and other periodic tasks
    }

    // ---------------------------------------------------------------------
    // Radar / IMU simulation (for debugging without hardware)
    // ---------------------------------------------------------------------
#ifdef SIMULATE_RADAR
    static float simX = 0.0f;
    static float simZ = 0.0f;
    char buffer[512];

    simX += 0.4f;
    simZ += 0.2f;
    if (simX > (100.0f / 1.414f)) simX = 0.0f;
    if (simZ > (100.0f / 0.707f)) simZ = 0.0f;

    // Simulated radar payload format: "speed,altX,altZ"
    snprintf(buffer, sizeof(buffer), "123.4,%f,%f", simX, simZ);
    doRadar(this, buffer, strlen(buffer));
    Radarstat = true;

    timerStart->start(100);   // faster timer while simulating

    static float t0 = 0.0f;
    float t = t0;
    t0 += 0.1f;               // 0.1 s step

    // Simulated attitude angles
    float roll  = 30.0f * sinf(2.0f * 3.14159f * 0.2f * t);
    float pitch = 20.0f * sinf(2.0f * 3.14159f * 0.1f * t);
    float yaw   = fmodf(t * 20.0f, 360.0f) - 180.0f;

    AngleX = roll;
    AngleY = pitch;
    AngleZ = yaw;
    IMUconnected = true;
    Transponderstat = true;
#endif  // SIMULATE_RADAR

#ifndef Q_OS_MAC
    setbacklit();     // Android: periodically force bright backlight
#endif
}

// ============================================================================
// Android display backlight helper
// ============================================================================

/**
 * @brief On Android, periodically set the external transponder display backlight.
 *
 * Uses Java class: com.hoho.android.usbserial.driver.TestClassTerje
 */
void MyTcpSocket::setbacklit()
{
#ifdef Q_OS_ANDROID   // Only Android version has the Java backlight hook
    static int disp = 999;

    if (someJavaObject == nullptr) {
        QJniEnvironment env;
        auto context = QJniObject(QNativeInterface::QAndroidApplication::context());

        // Check if Java class is available and construct helper object
        if (QJniObject::isClassAvailable(
                "com/hoho/android/usbserial/driver/TestClassTerje")) {
            someJavaObject = new QJniObject(
                "com/hoho/android/usbserial/driver/TestClassTerje",
                "(Landroid/content/Context;)V",
                context.object());
        }
    }

    // Every 40 ticks we send a "change brightness" command
    if (++disp > 40 && someJavaObject != nullptr) {
        disp = 0;
        int y = someJavaObject->callMethod<jint>("change", "(I)I", 255);
        qDebug() << "Display Backlit set to:" << y;
    }
#endif
}

// ============================================================================
// IMU / INS / Radar connection setup
// ============================================================================

/**
 * @brief Try to connect to IMU/INS and Radar devices.
 *
 * Order:
 *   1. Bluetooth IMU (WT901 BLE) if enabled.
 *   2. Serial IMU (WTGAHRS3).
 *   3. Radar (USB serial).
 */
void MyTcpSocket::connectedIMU()
{
#ifndef Q_OS_ANDROID
    // callbacks = new Callbacks();   // Left as a placeholder if needed
#endif

    // ---------------------------------------------------------------------
    // 1) Bluetooth IMU (WT901BLE67), if compiled with USE_BT_IMU
    // ---------------------------------------------------------------------
#if defined(USE_BT_IMU)
    NoButtonMessageBox *m_msgBoxIMU =
        new NoButtonMessageBox(tr("Looking for Bluetooth device WT901BLE67!"));
    m_msgBoxIMU->show();

    // Wait for BT scan to finish
    int timeout = 5*10;
    while (!bluetootPort->serial_->scancomplete) {
        QThread::msleep(200);
        QCoreApplication::processEvents();
        if(--timeout == 0) break;
    }

    if (bluetootPort->open("", 0)) {
        // Launch INS driver in BT mode
//        INS_driver(static_cast<void *>(this), nullptr, bluetootPort,reinterpret_cast<void *>(doIMU));
        INS_driver(static_cast<void *>(this), nullptr, bluetootPort,reinterpret_cast<void *>(parseIMU));

        IMUconnected = false;
        for (int delay = 0; delay < 8; ++delay) {
            QThread::msleep(200);
            QCoreApplication::processEvents();

            if (AutoScanSensor()) {
                IMUconnected = true;
                break;
            }
        }
    } else {
        qDebug() << "IMU NOT Connected (Bluetooth)...";
        IMUconnected = false;
    }

    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;

    // Show result for BT device
    if (!IMUconnected) {
        m_msgBoxIMU = new NoButtonMessageBox(
            tr("Bluetooth device WT901BLE67 not found!"));
    } else {
        m_msgBoxIMU = new NoButtonMessageBox(
            tr("Bluetooth device WT901BLE67 found and connected..."));
    }
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1500);
    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;
#endif  // USE_BT_IMU

    // ---------------------------------------------------------------------
    // 2) USB IMU / INS device WTGAHRS3 (serial)
    // ---------------------------------------------------------------------
#ifndef Q_OS_IOS
    NoButtonMessageBox *m_msgBoxIMUx = nullptr;

    if (!IMUconnected) {
        m_msgBoxIMUx = new NoButtonMessageBox(tr("Looking for USB device WTGAHRS1/WTGAHRS3!"));
        m_msgBoxIMUx->show();
        QCoreApplication::processEvents();
        QThread::msleep(500);

#ifdef Q_OS_ANDROID
        if (INSSerPort->open(_IMU_id, QSerialPort::Baud9600))
#else
        QString IMU_name = findPort(_IMU_id);
        qDebug() << "Looking for Port:" << IMU_name;
        if (!IMU_name.isEmpty() && INSSerPort->open(IMU_name, QSerialPort::Baud9600))
#endif
        {
            // Launch INS driver in serial mode
    //        INS_driver(static_cast<void *>(this), INSSerPort, nullptr,reinterpret_cast<void *>(doIMU));
            INS_driver(static_cast<void *>(this), INSSerPort, nullptr,reinterpret_cast<void *>(parseIMU));

            for (int delay = 0; delay < 10; ++delay) {
                if (AutoScanSensor()) {
                    // Optionally change baud rate to 115200 once recognized
                    // AutoSetBaud(QSerialPort::Baud115200);
                    IMUconnected = true;
                    break;
                }
                QThread::msleep(100);
                QCoreApplication::processEvents();
                if (delay == 9) {
                    IMUconnected = false;
                }
                if (delay == 5) {
                    // AutoSetBaud(QSerialPort::Baud115200);
                }
            }
        }

        QCoreApplication::processEvents();
        QThread::msleep(1500);

        m_msgBoxIMUx->hide();
        delete m_msgBoxIMUx;

        // IMU result message
        if (!IMUconnected) {
            m_msgBoxIMUx =
                new NoButtonMessageBox(tr("USB device WTGAHRS3 not found!"));
        } else {
            m_msgBoxIMUx =
                new NoButtonMessageBox(tr("USB device WTGAHRS3 found and connected..."));
        }
        m_msgBoxIMUx->show();
        QCoreApplication::processEvents();
        QThread::msleep(1500);
        m_msgBoxIMUx->hide();
        delete m_msgBoxIMUx;
    }

    // ---------------------------------------------------------------------
    // 3) Radar (USB serial)
    // ---------------------------------------------------------------------
    m_msgBoxIMU = new NoButtonMessageBox(tr("Looking for NRA24 RADAR!"));
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(500);

#ifdef Q_OS_ANDROID
    if (RadarSerPort->open(_radar_id, QSerialPort::Baud115200)) {
#else
    QString radar_name = findPort(_radar_id);
    qDebug() << "Looking for Radar Port:" << radar_name;
    if (radar_name.isEmpty()) {
        Radarstat = false;
    } else {
        if (RadarSerPort->open(radar_name, QSerialPort::Baud115200)) {
#endif
        Radarstat = true;
    } else {
        Radarstat = false;
    }
#ifndef Q_OS_ANDROID
}
#endif

    QCoreApplication::processEvents();
    QThread::msleep(1500);

    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;

    // Radar result message
    if (!Radarstat) {
        m_msgBoxIMU = new NoButtonMessageBox(tr("USB RADAR not found!"));
    } else {
        m_msgBoxIMU = new NoButtonMessageBox(tr("USB RADAR found and connected..."));
    }
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1500);
    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;
#endif  // !Q_OS_IOS
}

// ============================================================================
// Transponder connection
// ============================================================================

/**
 * @brief Try to connect transponder on the configured serial port.
 *
 * On success:
 *   - Sets Transponderstat = true
 *   - Queries version and configuration
 *   - Starts timerAlt to periodically call doTransponder()
 */
void MyTcpSocket::connected()
{
#ifndef Q_OS_IOS
#ifdef Q_OS_ANDROID
    if (TransponderSerPort->open(_transponder_id, QSerialPort::Baud9600))
#else
        QString transponder_name = findPort(_transponder_id);
        qDebug() << "Looking for Port:" << transponder_name;
        if (!transponder_name.isEmpty() &&
            TransponderSerPort->open(transponder_name, QSerialPort::Baud9600))
#endif
    {
        Transponderstat = true;

        // Initial version query
        readyWrite(const_cast<char *>("v=?\n"));
        QThread::usleep(500);

        // Setup periodic transponder polling
        if (timerAlt) {
            timerAlt->stop();
            delete timerAlt;
        }
        timerAlt = new QTimer(this);
        timerAlt->setSingleShot(false);
        connect(timerAlt, SIGNAL(timeout()), SLOT(doTransponder()));
        timerAlt->start(150);
    } else {
        Transponderstat = false;
    }
#endif
}

// ============================================================================
// IMU data handling
// ============================================================================

/**
 * @brief Static callback from INS_driver / WIT SDK.
 *
 * Parses ASCII key/value pairs such as:
 *   "AccX 0.123", "AngleY 10.0", "LAT 59.00", ...
 *
 * @param parent  Pointer back to MyTcpSocket instance (this).
 * @param data    Null-terminated ASCII string from the IMU.
 * @param length  Data length in bytes (unused here).
 */

static inline int32_t join32(uint16_t lo, uint16_t hi) {
    return (int32_t)(((uint32_t)hi << 16) | lo);
}

static double nmea_ddmm_to_deg(int32_t raw) {
    // raw is ddmm.mmmmmm with decimal removed; may be signed
    int sign = (raw < 0) ? -1 : 1;
    uint32_t v = (raw < 0) ? (uint32_t)(-raw) : (uint32_t)raw;

    uint32_t dd        = v / 10000000U;       // whole degrees
    uint32_t mm_x1e6   = v % 10000000U;       // minutes * 1e6
    double minutes     = mm_x1e6 / 100000.0;  // mm.mmmmmm

    double deg = (double)dd + minutes / 60.0;
    return sign * deg;
}

/*
 Inputs:
   vN, vE, vD   : velocity in NED (m/s)
   roll_deg     : φ  (right-wing down positive)  [deg]
   pitch_deg    : θ  (nose up positive)          [deg]
   yaw_deg      : ψ  (heading from North, CW to East) [deg]
 Output:
   LFD velocity components (Left, Forward, Down) in m/s
 Notes:
   Uses standard aerospace ZYX Euler convention:
     C_n_b = Rz(yaw) * Ry(pitch) * Rx(roll)  (body → NED)
   Then v_b = C_b_n * v_n = C_n_b^T * v_n
*/
// deg→rad helper
static inline double deg2rad(double deg){ return deg * M_PI / 180.0; }

// If your course is TRUE and yaw is MAGNETIC, pass decl_deg = +declination (east positive)
// to rotate magnetic -> true. If both already TRUE, pass 0.
static LFD body_vel_from_speed_course_yaw(double speed_ms,
                                          double course_deg_true,
                                          double yaw_deg,            // IMU heading
                                          double decl_deg,           // usually 0.0
                                          double vDown_ms)           // use 0.0 if unknown
{
    // Correct yaw if it’s magnetic but course is true
    double yaw_true = yaw_deg + decl_deg;

    // Relative angle between motion and body forward
    double d = deg2rad(course_deg_true - yaw_true);

    // Forward/Left in body frame (Down passed through)
    LFD out;
    out.F = speed_ms * cos(d);
    out.L = -speed_ms * sin(d);   // Left = -Right
    out.D = vDown_ms;             // If no vertical speed, set 0 or compute from alt-rate
    return out;
}

void MyTcpSocket::parseIMU(void *parent,uint32_t uiReg, uint16_t sRegAll[])
{
    auto *local = static_cast<MyTcpSocket *>(parent);

    if(uiReg == Roll)
    {
        const double   g     = 9.82500;

        local->AccX        = -1*((float)sReg[AX   + 0] / 32768.0f * 16.0f) * g;
        local->AccY        =    ((float)sReg[AX   + 1] / 32768.0f * 16.0f) * g;
        local->AccZ        =    ((float)sReg[AX   + 2] / 32768.0f * 16.0f) * g;

        local->AsZ         =    ((float)sReg[GX   + 0] / 32768.0f * 2000.0f);
        local->AsY         = -1*((float)sReg[GX   + 1] / 32768.0f * 2000.0f);
        local->AsX         =    ((float)sReg[GX   + 2] / 32768.0f * 2000.0f);

        local->AngleX      =    ((float)sReg[Roll + 0] / 32768.0f * 180.0f);
        local->AngleY      =    ((float)sReg[Roll + 1] / 32768.0f * 180.0f);
        local->AngleZ      = -1*((float)sReg[Roll + 2] / 32768.0f * 180.0f);

        local->HX          =     (float)sReg[HXi  + 0];
        local->HY          =     (float)sReg[HXi  + 1];
        local->HZ          =     (float)sReg[HXi  + 2];

        // GPS / altitude
        local->m_longitude = nmea_ddmm_to_deg(join32( sReg[LonL],sReg[LonH]));
        local->m_latitude = nmea_ddmm_to_deg(join32(sReg[LatL],sReg[LatH]));
        //        GPS[2] = (float)sReg[GPSHeight]/10.0; // Get altitude...
        local->m_altitude = (float)sReg[D0Status]/10.0; // Get altitude...

        local->Temp = (float) sReg[TEMP]/100.0;
        local->VER = sRegAll[VERSION];

        local->m_pressure_raw = join32(sReg[PressureL],sReg[PressureH])/100.0;
      //  local->m_preasure_alt     = join32(sReg[HeightL],sReg[HeightH])/100.0;

        uint32_t raw = join32(sReg[GPSVL], sReg[GPSVH])/100.0;
        int32_t yaw = sReg[GPSYAW] / 100.0;     // deg (use /10.0 if your fw is 0.1°/LSB)
        LFD vel = body_vel_from_speed_course_yaw(raw, yaw, local->AngleZ, 0,0);

        local->FW_Speed    =  vel.F;
        local->Donwn_Speed =  vel.D;  // always zero per comment
//        qDebug() <<  local->AngleX  << "  :  " <<  local->AngleY << "  :  " <<  local->AngleZ;
    }
}

// ============================================================================
// Radar data handling
// ============================================================================

/**
 * @brief Static callback for Radar serial data.
 *
 * Expected data format: "pos,radialSpeed,radialDist"
 * Performs simple geometry using a fixed azimuth.
 *
 * @param parent  Pointer back to MyTcpSocket instance.
 * @param data    Null-terminated ASCII CSV.
 * @param length  Data length.
 */
void MyTcpSocket::doRadar(void *parent, const char *data, uint32_t length)
{
    // Fixed radar azimuth (degrees)
    constexpr float kAzimuthDeg = 0.0f;  // 45.0

    auto *local = static_cast<MyTcpSocket *>(parent);
    if (length > 5) {
        QString str = QString::fromLatin1(data);
        const QList<QString> fields = str.split(',');

        if (fields.length() == 3) {
            const float azimuthRad = kAzimuthDeg / (180.0f / static_cast<float>(M_PI));
            const float cosAz      = cosf(azimuthRad);

            local->rPos++;
            local->rSpeed = fields[1].toFloat() / cosAz;
            local->rDist  = cosAz * fields[2].toFloat();
        }
    }
}

// ============================================================================
// Transponder polling state machine (timerAlt)
// ============================================================================

/**
 * @brief Periodically poll/configure the transponder.
 *
 * Simple state-machine that:
 *   - Asks for version, altitude, configuration, etc.
 *   - Optionally updates altitude based on barometer (if present).
 *
 * Called from timerAlt every ~150 ms.
 */
void MyTcpSocket::doTransponder()
{
    static bool dosend = false;
    static int  state  = 0;

    if (!dosend) {
        dosend = true;
        return;
    }

    dosend = false;

    if (!Transponderstat)
        return;

    switch (state) {
    case 0:
        readyWrite(const_cast<char *>("v=1\r\n"));
        break;
    case 1:
        readyWrite(const_cast<char *>("z=?\r\n"));
        break;
    case 2:
        readyWrite(const_cast<char *>("a=?\r\n"));
        break;
    case 3:
        readyWrite(const_cast<char *>("c=?\r\n"));
        break;
    case 4:
        readyWrite(const_cast<char *>("s=?\r\n"));
        break;
    case 5:
        readyWrite(const_cast<char *>("i=?\r\n"));
        break;
    case 6:
        readyWrite(const_cast<char *>("r=y\r\n"));
        break;
    case 7:
        readyWrite(const_cast<char *>("d=?\r\n"));
        break;
    case 8:
        // If transponder has internal barometer and we have a valid QNH,
        // push current altitude to transponder to correct known bug.
        if (TransponderstatWithBarometer && m_preasure_QNH != -10000) {
            char x[64];
            snprintf(x, sizeof(x), "a=%dM\r\n",
                     static_cast<int>(m_preasure_QNH * 0.3048));  // feet -> meters
            readyWrite(x);
        }
        break;
    default:
        break;
    }

    if (++state > 8) {
        state = 2;  // Loop among query states once initial setup is done
    }
}

// ============================================================================
// MQTT (X-Plane) message handling
// ============================================================================

/**
 * @brief Handle MQTT / X-Plane updates and map them to internal state.
 *
 * Topics currently supported (examples):
 *   - xplane/ax, xplane/ay, xplane/az       : accelerometer / gyro
 *   - xplane/roll, xplane/pitch, xplane/yaw : attitude
 *   - xplane/airspeed, xplane/climbRate     : air data
 *   - xplane/localPressure                  : barometric pressure
 *
 * @param ID     Topic string.
 * @param value  Parsed float payload.
 */
void MyTcpSocket::handleUpdate(const std::string &ID, float value)
{
    static bool first       = true;
    static int  statusCount = 0;

    const QString topic = QString::fromStdString(ID);
    qDebug() << "[MQTT]" << topic << "=" << value;

    if (ID == "xplane/topic") {
        qDebug() << "TOPIC:" << topic;
    } else if (ID == "xplane/ax") {
        m_has_MQTT_gyro = true;
        AsX = value;
        ++statusCount;
    } else if (ID == "xplane/ay") {
        m_has_MQTT_gyro = true;
        AsY = value;
        ++statusCount;
    } else if (ID == "xplane/az") {
        m_has_MQTT_gyro = true;
        AsZ = value;
        ++statusCount;
    } else if (ID == "xplane/rollRate") {
        m_has_MQTT_accel = true;
        ++statusCount;
    } else if (ID == "xplane/pitchRate") {
        m_has_MQTT_accel = true;
        ++statusCount;
    } else if (ID == "xplane/yawRate") {
        m_has_MQTT_accel = true;
        ++statusCount;
    } else if (ID == "xplane/climbRate") {
        m_has_MQTT_vsi = true;
        ++statusCount;
    } else if (ID == "xplane/heading") {
        m_has_MQTT_heading = true;
        //m_heading = value;
        ++statusCount;
    } else if (ID == "xplane/airspeed") {
        m_has_MQTT_airspeed = true;
        m_speed = value;
        ++statusCount;
    } else if (ID == "xplane/localPressure") {
        m_has_MQTT_preassure = true;
        ++statusCount;
    } else if (ID == "xplane/roll") {
        AngleX = value;
        ++statusCount;
    } else if (ID == "xplane/pitch") {
        AngleY = value;
        ++statusCount;
    } else if (ID == "xplane/yaw") {
        AngleZ = value;
        ++statusCount;
    }

    // Mark MQTT as usable once we have a minimum number of key values
    if (statusCount >= 4 && first) {
        first = false;
        m_has_MQTT = true;
    }
}

// ============================================================================
// Low-level write helper
// ============================================================================

/**
 * @brief Send a raw ASCII command to the transponder, if connected.
 *
 * @param data  Null-terminated C string (command).
 */
void MyTcpSocket::readyWrite(char *data)
{
    if (Transponderstat) {
        TransponderSerPort->send(data);
    }
}
