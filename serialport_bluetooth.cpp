#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
//#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include "serialport_bluetooth.h"

BTComQt::BTComQt(QObject *parent) : QObject(parent)
{
    this->parent = parent;
}

BTComQt::~BTComQt()
{
    close();
}

bool BTComQt::open(const QString &portName, qint32 baudrate)
{
    QJniEnvironment env;
    // C++ code

    auto context = QJniObject(QNativeInterface::QAndroidApplication::context());

    //check if program can find our java class and use it
    if(QJniObject::isClassAvailable("com/wit/witsdk/Device/IMUActivity"))
    {
        imuJavaObject = new QJniObject("com/wit/witsdk/Device/IMUActivity"
                                       ,"(Landroid/content/Context;)V",
                                       context.object());

//        qDebug() << "------------- Waiting for IMU Connected...";
        QString stat;
        for(int i=0; i < 30; i++)
        {
            stat = imuJavaObject->callMethod<jstring>("TestIMU").toString();
            if(stat == "true") break;
            QThread::msleep(200); // 400);
         //   QCoreApplication::processEvents();

        }
//        qDebug() << "------------- Done Waiting for IMU Connected...";

        if( stat == "true"){
            qDebug() << "connected device done ...";
            handleReadyRead();
            //use a timer to allow the constructor to exit
            timerAndroid = new QTimer(this);
            timerAndroid->setSingleShot(false);
            connect(timerAndroid, SIGNAL(timeout()), SLOT(handleReadyRead()));
            timerAndroid->start(20);
            return true;
        }
        else{
            qDebug() << "Device NOT Connected...";
        }
    }
    else {
        qDebug() << "SOME JAVA CLASS UNAVAIABLE!";
    }
    close();
    return false;
}

void BTComQt::close()
{
    QString x = imuJavaObject->callObjectMethod<jstring>("disconn").toString();
//    qDebug() << x;
}


bool BTComQt::send(const QByteArray &data)
{
    QJniObject _text = QJniObject::fromString(data);
    // jint x =
    int size = imuJavaObject->callMethod<jint>("sendToSerial", "(Ljava/lang/String;)I", _text.object() );
    if(size) return true;
    else return false;
}

bool BTComQt::send(const char *data, unsigned short len)
{
    return send(QByteArray::fromRawData(data, len));
}

void BTComQt::handleReadyRead()
{
    QString data = imuJavaObject->callMethod<jstring>("getIMU").toString();
    if(data.length() > 0){
        QByteArray ba=data.toUtf8();
        char *dta = ba.data();
        callback_(this->parent,dta,data.length()); //.data(), static_cast<uint32_t>(data.size()));
        //callback_(this->parent,ba); //.data(), static_cast<uint32_t>(data.size()));

        // And emit Qt signal for other consumers
        emit dataReceived(ba);
    }
}
