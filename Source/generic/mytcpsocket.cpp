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
#include <QHostAddress>
#include <QNetworkInterface>

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
#include "tcpclient.h"

#include <QVector>
#include <QString>
#include <QMap>
#include <unistd.h>   // usleep
#include "REG.h"
#include "ssdp.h"
//#include "multicastlistner.h"


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
    this->ret_transponder   = retx;  // C callback for transponder data
    this->ret_imu  = rety;  // C callback to notify IMU status
    this->text     = s;
    this->parent   = parent;

    //-------------------------------------------------------------
    // Set up the multicast listern to find wlan sensors...
    ssdpConfig();

    // ---------------------------------------------------------------------
    // Serial / Bluetooth COM objects
    // ---------------------------------------------------------------------
#ifndef Q_OS_IOS
    // Transponder serial port
    TransponderSerPort = new ComQt(parent);
    TransponderSerPort->setParent(this);
    TransponderSerPort->setRxCallback(ret_transponder);             // register C callback

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
#endif

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

    // ---------------------------------------------------------------------
    // Deferred startup using a timer (ensures constructor returns first)
    // ---------------------------------------------------------------------
    timerStart = new QTimer(this);
    timerStart->setSingleShot(false);
    connect(timerStart, SIGNAL(timeout()), this, SLOT(doStart()));
    timerStart->start(500);                             // first step after 200 ms

}

/**
 * @brief Destructor. Closes all serial ports.
 */
MyTcpSocket::~MyTcpSocket()
{
#ifndef Q_OS_IOS
    TransponderSerPort->close();
    RadarSerPort->close();
    INSSerPort->close();
#endif
    qDebug() << "Stopped socket...";
}

/**
 * @brief multicastConfig. Sends multicast...
 */
void MyTcpSocket::ssdpConfig()
{
    disc = new SsdpDiscoverer(this);

    connect(disc, &SsdpDiscoverer::deviceFound,
            this, [this](const QHostAddress &addr, quint16 port, const QString &st)
            {
        /*
                qDebug() << "SSDP device found:"
                         << "IP="  << addr.toString()
                         << "PORT="<< QString::number(port)
                         << "ST="  << st;
        */
                // ---------- Match device type by ST ----------
                if (st == "IMU") {
                    m_imu_address = addr.toString();
                    qDebug() << "IMU FOUND:" << m_imu_address;
                    if (m_imuClient) {
                    //    if (m_imuClient->state() == QAbstractSocket::UnconnectedState) {
                            m_imuClient->disconnectFromHost();
                        }
                    //}
                }
                else if (st == "RADAR") {
                    m_radar_address = addr.toString();
                    qDebug() << "RADAR FOUND:" << m_radar_address;
                    if (m_radarClient) {
                    //    if (m_radarClient->state() == QAbstractSocket::UnconnectedState) {
                            m_radarClient->disconnectFromHost();
                        }
                    //}
                }
                else if (st == "TRANSPONDER") {
                    m_transponder_address = addr.toString();
                    qDebug() << "TRANSPONDER FOUND:" << m_transponder_address;
                    if (m_transponderClient) {
                    //    if (m_transponderClient->state() == QAbstractSocket::UnconnectedState) {
                            m_transponderClient->disconnectFromHost();
                        }
                    //}
                }
                else {
                    qDebug() << "Unknown SSDP service type:" << st;
                }
            });
}

//====================================================================
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
    static int delay = 0;
    static int send_boot = 10;

    static int state = 0;
    timerStart->stop();

    switch(state){
    case 0:
        disc->startDiscovery(5000);  // listen for 5 seconds
        // Wait for a total of 2.5 seconds...
        if(send_boot++ > 20){
            send_boot = 0;
            ++state;
        }
        QCoreApplication::processEvents();
        break;
    case 1:
        connectedIMU();          // try to find & initialize IMU / INS
        ++state;
        break;
    case 2:
        connectedRadar();          // try to find & initialize IMU / INS
        ++state;
        break;
    case 3:
        connected();             // try to find & initialize transponder
        ++state;
        break;
    case 4:
        if(delay++ > 15){
            delay = 0;
            setbacklit();     // Android: periodically force bright backlight
        }
        /*
        switch(delay){
        case 5:
            if(!IMUconnected){
                connectedIMU();          // try to find & initialize IMU / INS
            }
        case 10:
            if(!Radarstat){
                connectedRadar();          // try to find & initialize IMU / INS
            }
        case 14:
            if (!Transponderstat){
                connected();
            }
        }
        */
        break;
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

    timerStart->start(500);
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
 *   2. Serial IMU WTGAHRS1/3.
 *   3. Wlan IMU WTGAHRS1/3.
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

    NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
        tr("Looking for Bluetooth device WT901BLE67 ..."));
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1000);
    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;

    // Wait for BT scan to finish
    int timeout = 5*10;
    while (!bluetootPort->serial_->scancomplete) {
        QThread::msleep(200);
        QCoreApplication::processEvents();
        if(--timeout == 0) break;
    }

    if (bluetootPort->open("", 0)) {
        // Launch INS driver in BT mode
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

    // Show result for BT device
    if (IMUconnected)
    {
        NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
            tr("Bluetooth device WT901BLE67 found and connected..."));
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(1000);
        m_msgBoxIMU->hide();
        delete m_msgBoxIMU;
    }
#endif  // USE_BT_IMU

    // ---------------------------------------------------------------------
    // 2) USB IMU / INS device WTGAHRS3 (serial)
    // ---------------------------------------------------------------------
#ifndef Q_OS_IOS
    if (!IMUconnected)
    {
        NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
            tr("Looking for Serial device WTGAHRS1/3 ..."));
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(1000);
        m_msgBoxIMU->hide();
        delete m_msgBoxIMU;

#ifdef Q_OS_ANDROID
        if (INSSerPort->open(_IMU_id, QSerialPort::Baud9600))
#else
        QString IMU_name = findPort(_IMU_id);
        qDebug() << "Looking for Port:" << IMU_name;
        if (!IMU_name.isEmpty() && INSSerPort->open(IMU_name, QSerialPort::Baud9600))
#endif
        {
            // Launch INS driver in serial mode
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

        // IMU result message
        if (IMUconnected) {
            NoButtonMessageBox *m_msgBoxIMUx = new NoButtonMessageBox(
                tr("Serial device WTGAHRS1/3 found and connected..."));
            m_msgBoxIMUx->show();
            QCoreApplication::processEvents();
            QThread::msleep(1000);
            m_msgBoxIMUx->hide();
            delete m_msgBoxIMUx;
        }
    }
#endif  // !Q_OS_IOS

    // --------------------------------------------------------------------------------
    // Check is the IMU sensor in connected on the wlan...
    if(IMUconnected == false)
    {
        NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
            tr("Looking for Networked device WTGAHRS1/3 ..."));
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(1000);
        m_msgBoxIMU->hide();
        delete m_msgBoxIMU;

        if(m_imu_address != "")
        {
            // somewhere in ctor or init:
            static void*_this = this;
            m_imuClient = new QTcpSocket(this);
            connect(m_imuClient, &QTcpSocket::connected, this, []() {
                qDebug() << "IMU TCP connected";
            });
            connect(m_imuClient, &QTcpSocket::disconnected, this, [this]() {
                m_imuClient->connectToHost(QHostAddress(m_imu_address), 23);
                qDebug() << "IMU TCP disconnected";
            });
            connect(m_imuClient, &QTcpSocket::readyRead, this, [this]() {
                QByteArray data = m_imuClient->readAll();
                if (!data.isEmpty()) {
                    WitSerialDataIn(this, data.constData(), data.size());
                }
            });

            m_imuClient->connectToHost(QHostAddress(m_imu_address), 23);
            m_imuClient->write("Host connected...\r\n");
            IMUconnected = true;

            INS_driver(static_cast<void *>(this), nullptr, nullptr,reinterpret_cast<void *>(parseIMU));

            NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
                tr("Found IMU on wlan!"));
            m_msgBoxIMU->show();
            QCoreApplication::processEvents();
            QThread::msleep(1000);
            m_msgBoxIMU->hide();
            QCoreApplication::processEvents();
            delete m_msgBoxIMU;

        }
    }

    // Notify IMU connection state through callback
    this->ret_imu(this->parent, IMUconnected);
}

void MyTcpSocket::connectedRadar()
{
    // ---------------------------------------------------------------------
    // 3) Radar (USB serial and NET)
    // ---------------------------------------------------------------------
#ifndef Q_OS_IOS

    NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
        tr("Looking for Serial device NanoRadar ..."));
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1000);
    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;

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

    // Radar result message
    if (Radarstat) {
        NoButtonMessageBox *m_msgBoxRadar = new NoButtonMessageBox(
            tr("Serial RADAR found and connected..."));
        QCoreApplication::processEvents();
        QThread::msleep(1000);
        m_msgBoxRadar->hide();
        delete m_msgBoxRadar;

    }
#endif

    if (!Radarstat)
    {
        NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
            tr("Looking for Networked device NanoRadar ..."));
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(1000);
        m_msgBoxIMU->hide();
        delete m_msgBoxIMU;

        if(m_radar_address != "")
        {
            // somewhere in ctor or init:
            static void*_this = this;
            m_radarClient = new QTcpSocket(this);
            connect(m_radarClient, &QTcpSocket::connected, this, []() {
                qDebug() << "Radar TCP connected";
            });
            connect(m_radarClient, &QTcpSocket::disconnected, this, [this]() {
                m_radarClient->connectToHost(QHostAddress(m_radar_address), 23);
                qDebug() << "Radar TCP disconnected";
            });
            connect(m_radarClient, &QTcpSocket::readyRead, this, [this]() {
                QByteArray data = m_radarClient->readAll();
                if (!data.isEmpty()) {
                    doRadar(this, data.constData(), data.size());
                }
            });
            m_radarClient->connectToHost(QHostAddress(m_radar_address), 23);
            m_radarClient->write("Host connected...\r\n");
            Radarstat = true;

            NoButtonMessageBox *m_msgBoxRadar = new NoButtonMessageBox(tr("Found Radar on wlan!"));
            m_msgBoxRadar->show();
            QCoreApplication::processEvents();
            QThread::msleep(1000);
            m_msgBoxRadar->hide();
            delete m_msgBoxRadar;
        }
    }
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

    NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
        tr("Looking for Serial device Transponder T2000 ..."));
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1000);
    m_msgBoxIMU->hide();
    delete m_msgBoxIMU;

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

        NoButtonMessageBox *m_msgBoxTrans = new NoButtonMessageBox(tr("Transponder found and connected..."));
        m_msgBoxTrans->show();
        QCoreApplication::processEvents();
        QThread::msleep(2000);
        m_msgBoxTrans->hide();
        delete m_msgBoxTrans;

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
    if (!Transponderstat)
    {
        NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(
            tr("Looking for Networked device Transponder T2000 ..."));
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(1000);
        m_msgBoxIMU->hide();
        delete m_msgBoxIMU;

        if(m_transponder_address != "")
        {
            // somewhere in ctor or init:
            static void*_this = this;
            m_transponderClient = new QTcpSocket(this);
            connect(m_transponderClient, &QTcpSocket::connected, this, []() {
                qDebug() << "Transponder TCP connected";
            });
            connect(m_transponderClient, &QTcpSocket::disconnected, this, [this]() {
                m_transponderClient->connectToHost(QHostAddress(m_transponder_address), 23);
                qDebug() << "Transponder TCP disconnected";
            });
            connect(m_transponderClient, &QTcpSocket::readyRead, this, [this]() {
                QByteArray data = m_radarClient->readAll();
                if (!data.isEmpty()) {
                    this->ret_transponder(this->parent, data, data.length());
                }
            });
            m_transponderClient->connectToHost(QHostAddress(m_transponder_address), 23);
            m_transponderClient->write("Host connected...\r\n");
            Transponderstat = true;

            NoButtonMessageBox *m_msgBoxTransponder = new NoButtonMessageBox(tr("Found Transponder on wlan!"));
            m_msgBoxTransponder->show();
            QCoreApplication::processEvents();
            QThread::msleep(1000);
            m_msgBoxTransponder->hide();
            delete m_msgBoxTransponder;

        }
    }
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
#define DEG2RAD (M_PI / 180.0)

LFD body_vel_from_speed_course_yaw(double speed,
                                   double course_deg,
                                   double body_yaw_deg,
                                   double vertical_speed,
                                   double /*unused*/)
{
    LFD v{};

    // Convert to radians
    double chi = course_deg * DEG2RAD;      // GPS course
    double psi = body_yaw_deg * DEG2RAD;    // body yaw

    // Ground velocity in NED
    double vN = speed * cos(chi);
    double vE = speed * sin(chi);

    // Rotate NED → body frame
    v.F =  cos(psi) * vN + sin(psi) * vE;   // Forward
    v.L = -sin(psi) * vN + cos(psi) * vE;   // Left/Right
    v.D = vertical_speed;                   // Down

    return v;
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
        local->VER = sReg[VERSION];

        local->m_pressure_raw = join32(sReg[PressureL],sReg[PressureH])/100.0;
      //  local->m_preasure_alt     = join32(sReg[HeightL],sReg[HeightH])/100.0;

        auto wrap360 = [](double deg) {
            deg = fmod(deg, 360.0);
            if (deg < 0) deg += 360.0;
            return deg;
        };

        // Treat registers as UNSIGNED 16-bit before joining
        uint32_t raw100 = (uint32_t)(uint16_t)sReg[GPSVL]
                          | ((uint32_t)(uint16_t)sReg[GPSVH] << 16);

        // Now convert to float speed (whatever unit you encoded)
        float speed = raw100 / 100.0f;
/*
        qDebug() << "GPSVL=" << QString("0x%1").arg((uint16_t)sReg[GPSVL],4,16,QChar('0'))
                 << "GPSVH=" << QString("0x%1").arg((uint16_t)sReg[GPSVH],4,16,QChar('0'))
                 << "raw100=" << raw100
                 << "speed=" << speed;
*/
        //double speed = (double)join32(sReg[GPSVL], sReg[GPSVH]) / 100.0;   // <-- double, not uint32_t
        double cog   = (double)sReg[GPSYAW] / 100.0;                      // course over ground (deg)
        double hdg   = (double)local->AngleZ;                             // your body heading (deg)

        // If AngleZ is "negative heading" (as your parser does), you may need:
        // hdg = -hdg;

        cog = wrap360(cog);
        hdg = wrap360(hdg);

        LFD vel = body_vel_from_speed_course_yaw(speed, cog, hdg, 0.0, 0.0);

        local->FW_Speed    = vel.F;
        local->Donwn_Speed = vel.D;
        local->m_gpsspeed  = vel.F;
        local->m_speed     = speed; //abs(vel.F);
       // qDebug() << "GPSVL=" << local->m_speed;
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
        // If transponder has internal barometer and we have a valid reading,
        // push current altitude to transponder to correct known bug.
        if (TransponderstatWithBarometer && m_pressure_raw > 1.0)
        {
            char x[64];
            snprintf(x, sizeof(x), "a=%dM\r\n",
                     static_cast<int>(m_pressure_raw * 0.3048));  // feet -> meters
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
        if(m_transponderClient != nullptr){
//            qDebug() << "Sending: " << data;
            m_transponderClient->write(data);
        }
        else{
#ifndef Q_OS_IOS
            TransponderSerPort->send(data);
#endif
        }
    }
}
