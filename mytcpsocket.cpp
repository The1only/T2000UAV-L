// mytcpsocket.cpp
#define UDP

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


#undef SIMULATE_RADAR

extern bool INS_driver(void *,ComQt *serPorts, ComBt *serPortb, void *func);
extern bool AutoScanSensor();
extern void AutoSetBaud(int);

MyTcpSocket::MyTcpSocket(QObject *parent,  QPlainTextEdit *s, void (*retx)(void *, const char *, uint32_t), void (*rety)(void *, bool use_imu)) :
    QObject(parent)
{
    this->ret = retx; // callback function with data...
    this->ret_imu = rety;
    this->text = s;
    this->parent = parent;

    TransponderSerPort = new ComQt(parent);
    TransponderSerPort->setParent(this);
    TransponderSerPort->setRxCallback(ret); // register C callback

    RadarSerPort  = new ComQt(this);
    RadarSerPort->setParent(this);
    RadarSerPort->setRxCallback(doRadar); // register C callback

    INSSerPort  = new ComQt(parent);
    INSSerPort->setParent(this);
    INSSerPort->setRxCallback(WitSerialDataIn); //ComRxCallBack); // register C callback

    bluetootPort = new ComBt(this);
    bluetootPort->setParent(this);
    bluetootPort->setRxCallback(WitSerialDataIn); //ComRxCallBack); // register C callback

    //use a timer to allow the constructor to exit
    timerStart = new QTimer(this);
    timerStart->setSingleShot(false);
    connect(timerStart, SIGNAL(timeout()), this, SLOT(doStart()));
    timerStart->start(100);

#ifdef Q_OS_MAC
    map = serialToPortMap();
    qDebug() << map;
#endif
}

MyTcpSocket::~MyTcpSocket()
{
    TransponderSerPort->close();
    RadarSerPort->close();
    INSSerPort->close();

    qDebug() << "Stoped socket...";
}

void MyTcpSocket::setSerialPorts(QString imu, QString transponder, QString radar)
{
    _transponder_id = transponder;
    _radar_id = radar;
    _IMU_id = imu;
}

#ifdef Q_OS_MAC

// Return a vector of PortEntry (one per detected port)
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

// Return a map: SerialNumber -> systemLocation (or portName)
QMap<QString, QString> MyTcpSocket::serialToPortMap(bool useSystemLocation)
{
    static QString portNum = "";
    static int portnum = 0;

    QMap<QString, QString> map;
    for (const auto &e : listSerialPortsDetailed()) {
        if (!e.portName.isEmpty() && !e.serial.isEmpty()) {
/*            if(e.serial.isEmpty()){
                portNum =  QString::number(portnum++);
            }
            else{ */
                portNum = e.serial;
//            }

            map.insert(portNum, useSystemLocation ? e.systemLocation : e.portName);

            qDebug() << "SerialPort: " <<  e.systemLocation << "\n";
            QSerialPort *serial_ = nullptr;
            serial_=new QSerialPort();
            serial_->setPortName(e.systemLocation);

            try {
                if (!serial_->open(QIODevice::ReadWrite))
                {
                    qWarning() << "Failed to open" << e.systemLocation << ":" << serial_->errorString();
                }
                if (serial_->isOpen())
                {
                    try {
                        serial_->close();
                        } catch (const std::exception& e) {
                            qWarning() << "Error while closing serial port:" << "Error:" << e.what();
                        }
                }
            } catch (const std::exception& e) {
                qWarning() << "Can not open serial port:" << "Error:" << e.what();
            }
            delete(serial_);
        }
    }
    qDebug() << map;
    return map;
}


QString MyTcpSocket::findPort(QString targetSerial)
{
    if (map.contains(targetSerial)) {
        return map.value(targetSerial);
    } else {
        qWarning() << "Device with serial" << targetSerial << "not found";
    }
    return "";
}
#endif

void MyTcpSocket::doStart(){
    static int state = 0;

    if(state == 0)
    {
        timerStart->stop();
        connectedIMU();
        timerStart->start(2000);
        state++;
    }
    else if(state == 1)
    {
        timerStart->stop();

        NoButtonMessageBox *m_msgBoxTrans = new NoButtonMessageBox(tr("Looking for USB Transponder!"));
        m_msgBoxTrans->show();
        QCoreApplication::processEvents();

        connected();

        QCoreApplication::processEvents();
        QThread::msleep(2000);

        m_msgBoxTrans->hide();
        delete(m_msgBoxTrans);

        if(Transponderstat == false){
            // Update message dynamically
            m_msgBoxTrans = new NoButtonMessageBox(tr("Transponder not found!"));
        }else{
            m_msgBoxTrans = new NoButtonMessageBox(tr("Transponder found and connected..."));
        }
        m_msgBoxTrans->show();
        QCoreApplication::processEvents();
        QThread::msleep(2000);
        m_msgBoxTrans->hide();
        delete(m_msgBoxTrans);

        this->ret_imu(this->parent, IMUconnected);
        timerStart->start(5000);
        state++;
    }
    else if(state == 2)
    {
        if( Transponderstat == false)
        {
            connected();
        }
//        else{
 //           state++;
//        }
//        timerStart->stop();   // Leave it running to set backlit several times, needs to be verified...
    }

    //------------------------------------------
    // For Radar simulation...
#ifdef SIMULATE_RADAR
    static int x= 0.0;
    char y[512];
    x = (x+10)%150;
    //                             speed,       alt
    snprintf(y,512,"123.4,%f,%f",(float)x,(float)x);
    doRadar(this, y, strlen(y));
    Radarstat = true;
    timerStart->start(100);

    static float t0 = 0; //QDateTime::currentMSecsSinceEpoch();
    float t = t0; //(QDateTime::currentMSecsSinceEpoch() - t0); // * 0.001f; // seconds
    t0+= 0.1;

    // --- Angles (0x53) ---
    float roll  = 30.0f * sinf(2.0f * 3.14159f * 0.2f * t);   // ±5°
    float pitch = 20.0f * sinf(2.0f * 3.14159f * 0.1f * t);   // ±3°
    float yaw   = fmodf(t * 20.0f, 360.0f) - 180.0f;         // -180..+180°, rotates ~20°/s

    AngleX=roll;
    AngleY=pitch;
    AngleZ=yaw;
    IMUconnected = true;

    Transponderstat = true;

#endif
    //------------------------------------------

#ifndef Q_OS_MAC
    setbacklit();
#endif
}

void MyTcpSocket::setbacklit()
{
#ifdef Q_OS_ANDROID   // Only the Android version got the Transponder...
    static int disp = 999;

    //-----------------------------------------
    if( someJavaObject == nullptr)
    {
        QJniEnvironment env;
        auto context = QJniObject(QNativeInterface::QAndroidApplication::context());

        //check if program can find our java class and use it
        if(QJniObject::isClassAvailable("com/hoho/android/usbserial/driver/TestClassTerje"))
        {
            someJavaObject = new QJniObject("com/hoho/android/usbserial/driver/TestClassTerje","(Landroid/content/Context;)V",context.object());
        }
    }

    if(++disp > 40 && someJavaObject != nullptr){
        disp = 0;
        int y = someJavaObject->callMethod<jint>("change", "(I)I", 255);
        qDebug() << "Display Backlit set to: " << y;
    }
#endif
}

void MyTcpSocket::connectedIMU()
{
#ifndef Q_OS_ANDROID
//    callbacks = new Callbacks();
#endif

    //-----------------------------------------
#if defined(USE_BT_IMU)
    NoButtonMessageBox *m_msgBoxIMU = new NoButtonMessageBox(tr("Looking for Bluetooth device WT901BLE67!"));
    m_msgBoxIMU->show();

    while(bluetootPort->serial_->scancomplete == false )
    {
        QThread::msleep(500);
        QCoreApplication::processEvents();
    }

    if(bluetootPort->open("", 0))
    {
        INS_driver((void*)this,nullptr,bluetootPort,(void*)doIMU);

        for(int delay = 0; delay < 5; delay++)
        {
            if(AutoScanSensor() == true){
                IMUconnected = true;
                break;
            }
            QThread::msleep(100);
            QCoreApplication::processEvents();
            if(delay == 4){
                IMUconnected = false;
            }
        }
    }
    else{
        qDebug() << "IMU NOT Connected...";
        IMUconnected = false;
    }
#endif

    m_msgBoxIMU->hide();
    delete(m_msgBoxIMU);

    if(IMUconnected == false){
        // Update message dynamically
        m_msgBoxIMU = new NoButtonMessageBox(tr("Bluetooth device WT901BLE67 not found!"));
    }else{
        m_msgBoxIMU = new NoButtonMessageBox(tr("Bluetooth device WT901BLE67 found and connected..."));
    }
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1500);
    m_msgBoxIMU->hide();
    delete(m_msgBoxIMU);

    //-----------------------------------------
    // Add INS Sensor on serial port...
#ifndef Q_OS_IOS
    if( IMUconnected == false)
    {
        m_msgBoxIMU = new NoButtonMessageBox(tr("Looking for USB device WTGAHRS3!"));
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(500);

#ifdef Q_OS_ANDROID
        if(INSSerPort->open(_IMU_id, QSerialPort::Baud9600))
#else
        QString IMU_name = findPort(_IMU_id);
        qDebug() << "Looking for Port: " << IMU_name;
        if(IMU_name != "") if(INSSerPort->open(IMU_name, QSerialPort::Baud9600))
#endif
        {
            INS_driver((void*)this,INSSerPort,nullptr,(void*)doIMU);
            for(int delay = 0; delay < 10; delay++)
            {
                if(AutoScanSensor() == true){
                 //   AutoSetBaud(QSerialPort::Baud115200);
                    IMUconnected = true;
                    break;
                }
                QThread::msleep(100);
                QCoreApplication::processEvents();
                if(delay == 9){
                    IMUconnected = false;
                }
                if(delay == 5){
               //   AutoSetBaud(QSerialPort::Baud115200);
                }
            }
        }

        QCoreApplication::processEvents();
        QThread::msleep(1500);

        m_msgBoxIMU->hide();
        delete(m_msgBoxIMU);

        if(IMUconnected == false){
            // Update message dynamically
            m_msgBoxIMU = new NoButtonMessageBox(tr("USB device WTGAHRS3 not found!"));
        }else{
            m_msgBoxIMU = new NoButtonMessageBox(tr("USB device WTGAHRS3 found and connected..."));
        }
        m_msgBoxIMU->show();
        QCoreApplication::processEvents();
        QThread::msleep(1500);
        m_msgBoxIMU->hide();
        delete(m_msgBoxIMU);

    }
    //-----------------------------------------

    m_msgBoxIMU = new NoButtonMessageBox(tr("Looking for USB RADAR!"));
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(500);

#ifdef Q_OS_ANDROID
    if(RadarSerPort->open(_radar_id, QSerialPort::Baud115200))
    {
#else
    // Add RADAR Sensor on serial port...
    QString radar_name = findPort(_radar_id);
    qDebug() << "Lookingfor Radar Port: " << radar_name;
    if(radar_name == ""){
        Radarstat = false;
    }
    else{
        if(RadarSerPort->open(radar_name, QSerialPort::Baud115200))
        {
#endif
            Radarstat = true;
        }
        else{
            Radarstat = false;
        }
#ifndef Q_OS_ANDROID
    }
#endif
#endif
    QCoreApplication::processEvents();
    QThread::msleep(1500);

    m_msgBoxIMU->hide();
    delete(m_msgBoxIMU);

    if(Radarstat == false){
        // Update message dynamically
        m_msgBoxIMU = new NoButtonMessageBox(tr("USB RADAR not found!"));
    }else{
        m_msgBoxIMU = new NoButtonMessageBox(tr("USB RADAR found and connected..."));
    }
    m_msgBoxIMU->show();
    QCoreApplication::processEvents();
    QThread::msleep(1500);
    m_msgBoxIMU->hide();
    delete(m_msgBoxIMU);
}

/*
void MyTcpSocket::SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
    INSSerPort->send((const char*)p_data,uiSize);
}
*/
void MyTcpSocket::connected()
{
#ifndef Q_OS_IOS
#ifdef Q_OS_ANDROID
    if(TransponderSerPort->open(_transponder_id,QSerialPort::Baud9600))
#else
    // Add Transponder on serial port...
    QString transponder_name = findPort(_transponder_id);
    qDebug() << "Looking for Port: " << transponder_name;
    if(transponder_name != "")
    {
        if(TransponderSerPort->open(transponder_name, QSerialPort::Baud9600))
#endif
        {
            Transponderstat = true;
            readyWrite((char*)"v=?\n");
            QThread::usleep(500);

            //use a timer to allow the constructor to exit
            if(timerAlt){ timerAlt->stop(); delete(timerAlt); }
            timerAlt = new QTimer(this);
            timerAlt->setSingleShot(false);
            connect(timerAlt, SIGNAL(timeout()), SLOT(doAlt()));
            timerAlt->start(150);
        }
        else {
            Transponderstat = false;
        }
#ifndef Q_OS_ANDROID
    }
    else {
        Transponderstat = false;
    }
#endif
#endif
}

void MyTcpSocket::doIMU(void *parent, const char *data, uint32_t length) //const QByteArray &data)
{
    MyTcpSocket* local = (MyTcpSocket*)parent;

    local->imuData = data;
  //  qDebug() << local->imuData;
    local->imuData.replace('\n','\t');
    QStringList pieces = local->imuData.split( "\t");
    if(pieces.length() > 1)
    {
        for(QString value : pieces)
        {
            QStringList element = value.split(" ");
            if(element.at(0) == "AccX"){
                local->AccZ = -element.at(1).toDouble()* 9.82500f;
            }
            if(element.at(0) == "AccY"){
                local->AccY = element.at(1).toDouble()* 9.82500f;
             //   qDebug() << "AccY: " <<local->AccY;
            }
            if(element.at(0) == "AccZ"){
                local->AccX = element.at(1).toDouble()* 9.82500f;
            }
            if(element.at(0) == "AsX"){
                local->AsZ = element.at(1).toDouble();
            }
            if(element.at(0) == "AsY"){
                local->AsY = -element.at(1).toDouble();
            }
            if(element.at(0) == "AsZ"){
                local->AsX = element.at(1).toDouble();
            }
            if(element.at(0) == "AngleX"){
                local->AngleX = element.at(1).toDouble();
            }
            if(element.at(0) == "AngleY"){
                local->AngleY = element.at(1).toDouble();
            }
            if(element.at(0) == "AngleZ"){
                local->AngleZ = -element.at(1).toDouble();
            }
            if(element.at(0) == "HX"){
                local->HX = element.at(1).toDouble();
            }
            if(element.at(0) == "HY"){
                local->HY = element.at(1).toDouble();
            }
            if(element.at(0) == "HZ"){
                local->HZ = element.at(1).toDouble();
            }
            if(element.at(0) == "VER"){
                local->VER = element.at(1).toDouble();
            }
            if(element.at(0) == "TEMP"){
                local->Temp = element.at(1).toDouble();
            }
            if(element.at(0) == "LAT"){
                local->LAT = element.at(1).toDouble();
            }
            if(element.at(0) == "LON"){
                local->LON = element.at(1).toDouble();
            }
            if(element.at(0) == "ALT"){
                local->ALT = element.at(1).toDouble();
            }
            if(element.at(0) == "PRES"){
                local->AIR_PRESSURE = element.at(1).toDouble();
            }
            if(element.at(0) == "BALT"){
                local->BARO_ALT = element.at(1).toDouble();
            }
            if(element.at(0) == "FW"){
                local->FW_Speed = element.at(1).toDouble();
            }
            if(element.at(0) == "tDOWN"){  // PS: Always zero...
                local->Donwn_Speed = element.at(1).toDouble();
            }
        }
    }
}

void MyTcpSocket::doRadar(void *parent, const char *data, uint32_t length)
{
#define azimuth 45.0

    MyTcpSocket* local = (MyTcpSocket*)parent;
    if(length > 5)
    {
        QString x = data;
        QList y = x.split(",");

        if(y.length() == 3)
        {
            local->rPos  = y[0].toFloat();
            local->rSpeed = cos(azimuth/(180.0/M_PI))*y[1].toFloat();
            local->rDist  = sin(azimuth/(180.0/M_PI))*y[2].toFloat();
        }
    }
}

void MyTcpSocket::doAlt()
{
    static bool dosend=false;
    static int state=0;

    if(dosend)
    {
        dosend= false;
        if( Transponderstat == true)
        {
            switch(state)
            {
                case 0:
                    this->readyWrite((char*)"v=1\r\n");
                    break;
                case 1:
                    this->readyWrite((char*)"z=?\r\n");
                    break;
                case 2:
                    this->readyWrite((char*)"a=?\r\n");
                    break;
                case 3:
                    this->readyWrite((char*)"c=?\r\n");
                    break;
                case 4:
                    this->readyWrite((char*)"s=?\r\n");
                    break;
                case 5:
                    this->readyWrite((char*)"i=?\r\n");
                    break;
                case 6:
                    this->readyWrite((char*)"r=y\r\n");
                    break;
                case 7:
                    this->readyWrite((char*)"d=?\r\n");
                    break;

                // If we got a barometer built inn, then use this to set altitude to transponder,
                // this is done to wix a bug that makes the altitude become bad...
                case 8:
                    if( TransponderstatWithBarometer == true && m_preasure_QNH != -10000)
                    {
                        char x[64];
                        snprintf(x, sizeof(x), "a=%dM\r\n", (int)(m_preasure_QNH*0.3048));  // adjust format as needed
                        this->readyWrite(x);
                    }
            }
            if(++state > 8){state = 2;}
        }
    }
    else
        dosend=true;
}

void MyTcpSocket::readyWrite(char *data)
{
    if( Transponderstat == true)
    {
        TransponderSerPort->send(data);
    }
}
