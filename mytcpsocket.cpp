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
// #include "IOS_swift/WitSDK/Sensor/Modular/Processor/Roles/BWT901BLE5_0DataProcessor.swift"
#endif

#ifdef Q_OS_MAC
//#include "BWT901BLE5_0DataProcessor.swift"
#endif

#include <QList>
#include <QCoreApplication>

#include "mytcpsocket.h"

MyTcpSocket::MyTcpSocket(QObject *parent,  QPlainTextEdit *s, void (*retx)(void *,QByteArray), void (*rety)(void *, bool use_imu)) :
    QObject(parent)
{
    this->ret = retx; // callback function with data...
    this->ret_imu = rety;
    this->text = s;
    this->parent = parent;

    //use a timer to allow the constructor to exit
    timerStart = new QTimer(this);
    timerStart->setSingleShot(false);
    connect(timerStart, SIGNAL(timeout()), this, SLOT(doStart()));
    timerStart->start(1000);
}

void MyTcpSocket::doStart(){
    static int state = 0;

    if(state == 0){
        connectedIMU();
        state++;

#ifndef Q_OS_ANDROID   // Only the Android version got the Transponder...
        state++;
#endif
    }
    else if(state == 1){
        connected();
        if( Transponderstat == "true")
        {
            state++;
        }
    }
    else{
//        timerStart->stop();
    }

    setbacklit();
    setbacklit();
}

void MyTcpSocket::setbacklit()
{
#ifdef Q_OS_ANDROID   // Only the Android version got the Transponder...
    static int disp = 0;
    if(++disp > 10 && someJavaObject != nullptr){
        disp = 0;
        int y = someJavaObject->callMethod<jint>("change","(I)I",215);
        //        qDebug() << "Display Backlit set to: " << y;
    }
#endif
}

void MyTcpSocket::connectedIMU()
{
#ifndef Q_OS_ANDROID
    callbacks = new Callbacks();

//    BWT901BLE5_0DataProcessor::SomeFunc();

#endif
#if defined(USE_BT_IMU) && defined(Q_OS_ANDROID)

    QJniEnvironment env;
    // C++ code

    auto context = QJniObject(QNativeInterface::QAndroidApplication::context());

    //check if program can find our java class and use it
    if(QJniObject::isClassAvailable("com/wit/witsdk/Device/IMUActivity"))
    {
        imuJavaObject = new QJniObject("com/wit/witsdk/Device/IMUActivity"
                                       ,"(Landroid/content/Context;)V",
                                       context.object());

        qDebug() << "------------- Waiting for IMU Connected...";
        QString stat;
        for(int i=0; i < 30; i++)
        {
            stat = imuJavaObject->callMethod<jstring>("TestIMU").toString();
            if(stat == "true") break;
            QThread::msleep(400);
        }
        qDebug() << "------------- Done Waiting for IMU Connected...";

        if( stat == "true"){
            qDebug() << "IMU Connected...";

            //use a timer to allow the constructor to exit
            timerIMU = new QTimer(this);
            timerIMU->setSingleShot(false);
            connect(timerIMU, SIGNAL(timeout()), this, SLOT(doIMU()));
            timerIMU->start(10);
            IMUconnected = true;
        }
        else{
            qDebug() << "IMU NOT Connected...";
        }
    }
    else {
        qDebug() << "SOME JAVA CLASS UNAVAIABLE!";
    }
    qDebug() << "connectedIMU done ...";
#endif
    this->ret_imu(this->parent, IMUconnected);
}


void MyTcpSocket::connected()
{
#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    // C++ code

    auto context = QJniObject(QNativeInterface::QAndroidApplication::context());

    //check if program can find our java class and use it
    if(QJniObject::isClassAvailable("com/hoho/android/usbserial/driver/TestClassTerje"))
    {
        someJavaObject = new QJniObject("com/hoho/android/usbserial/driver/TestClassTerje"
                                        ,"(Landroid/content/Context;)V",
                                        context.object());

    //  Seems screen dimming is unstabile, do not know if this helps...
    //    someJavaObject->callMethod<jint>("change","(I)I",255);

        Transponderstat = someJavaObject->callMethod<jstring>("getconnected").toString();
        //    qDebug() << "Serial port status= " + Transponderstat;

        if( Transponderstat == "true"){
            this->isconnected = true;
            QJniObject _text = QJniObject::fromString("v=?");
            jint x = someJavaObject->callMethod<jint>("sendToSerial", "(Ljava/lang/String;)I", _text.object() );
            qDebug() << x;

            x = someJavaObject->callMethod<jint>("ControlLines", "(ZZ)I",true, true );
            qDebug() << x;
            QThread::usleep(100);
            x = someJavaObject->callMethod<jint>("ControlLines", "(ZZ)I",false, false );
            qDebug() << x;

            QThread::usleep(1000);

            //use a timer to allow the constructor to exit
            timerAlt = new QTimer(this);
            timerAlt->setSingleShot(false);
            connect(timerAlt, SIGNAL(timeout()), this, SLOT(doAlt()));
            timerAlt->start(150);
        }

// For debug...
#ifdef QT_DEBUG
        Transponderstat = "true";
#endif

    }
    else {
        qDebug() << "SOME JAVA CLASS UNAVAIABLE!";
    }
#endif
}

MyTcpSocket::~MyTcpSocket()
{
#ifdef Q_OS_ANDROID
    QString x = someJavaObject->callObjectMethod<jstring>("disconn").toString();
    qDebug() << x;
#endif
    qDebug() << "Stoped socket...";
}


void MyTcpSocket::showMessage(const QString &sender, const QString &message) const
{
    Q_UNUSED(sender);
    QByteArray tmp = message.toUtf8();
    tmp.append('\0');
    this->ret(this->parent,tmp);
}


void MyTcpSocket::doIMU()
{
#ifdef Q_OS_ANDROID
    this->imuData = imuJavaObject->callMethod<jstring>("getIMU").toString();
#endif
    this->imuData.replace('\n','\t');
    QStringList pieces = this->imuData.split( "\t");
    if(pieces.length() > 1){
        for(QString value : pieces){
            QStringList element = value.split(" ");
            if(element.at(0) == "AccX"){
                this->AccX = element.at(1).toDouble()*G;
            }
            if(element.at(0) == "AccY"){
                this->AccY = element.at(1).toDouble()*G;
            }
            if(element.at(0) == "AccZ"){
                this->AccZ = element.at(1).toDouble()*G;
            }
            if(element.at(0) == "AsX"){
                this->AsZ = element.at(1).toDouble();
            }
            if(element.at(0) == "AsY"){
                this->AsY = element.at(1).toDouble();
            }
            if(element.at(0) == "AsZ"){
                this->AsX = element.at(1).toDouble();
            }
            if(element.at(0) == "AngleX"){
                this->AngleX = element.at(1).toDouble();
            }
            if(element.at(0) == "AngleY"){
                this->AngleY = element.at(1).toDouble();
            }
            if(element.at(0) == "AngleZ"){
                this->AngleZ = -element.at(1).toDouble();
            }
            if(element.at(0) == "HX"){
                this->HX = element.at(1).toDouble();
            }
            if(element.at(0) == "HY"){
                this->HY = element.at(1).toDouble();
            }
            if(element.at(0) == "HZ"){
                this->HZ = element.at(1).toDouble();
            }
            if(element.at(0) == "Electricity"){
                this->Electricity = element.at(1).toDouble();
            }
            if(element.at(0) == "Temperature"){
                this->Temperature = element.at(1).toDouble();
            }
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
        if(this->isconnected == true){
            switch(state){
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
            }

            if(++state > 6){
                state = 2;
            }
        }
    }
    else
        dosend=true;

#ifdef Q_OS_ANDROID
    QString receive = someJavaObject->callMethod<jstring>("recFromSerial").toString();
    if(receive.length() > 0){
        QByteArray ba=receive.toUtf8();
        this->ret(this->parent,ba);
    }
#endif
}

void MyTcpSocket::readyWrite(char *data)
{
    if(this->isconnected == true)
    {
#ifdef Q_OS_ANDROID
        QJniObject _text = QJniObject::fromString(data);
        // jint x =
        someJavaObject->callMethod<jint>("sendToSerial", "(Ljava/lang/String;)I", _text.object() );
#endif
    }
}
