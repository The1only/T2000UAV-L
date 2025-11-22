#include <QDebug>
#include "bleuart.h"   // your class header
#include <unistd.h>   // usleep

ComBt::ComBt(QObject *Parent) : QObject(Parent)
{
    this->parent = Parent;

    serial_= new BleUart();


    // Forward incoming BLE bytes to our API
    QObject::connect(serial_, &BleUart::bytesReceived, this, [this](const QByteArray &data){
      //  emit dataReceived(data);
//        qDebug() << "RX:" << data;
      //  qDebug() << "RX:" << data.toHex(' ');
//        char *dta = data.data();
        callback_(this->parent,data,data.length()); //.data(), static_cast<uint32_t>(data.size()));

    });
    // Track connection state
    QObject::connect(serial_, &BleUart::ready, this, [this](){
        open_ = true;
        emit connectionChanged(true);
    });
    QObject::connect(serial_, &BleUart::disconnected, this, [this](){
        open_ = false;
       // serial_->scancomplete = false;
        emit connectionChanged(false);
    });

    qDebug() << "Starting BLE scan...";
    serial_->startScan(5000, -95);

//    int ms = 60000;
//    usleep((useconds_t)ms * 1000u);
}

ComBt::~ComBt()
{
}

bool ComBt::open(const QString &portName, qint32 baudrate)
{
    (void) portName;
    (void) baudrate;
    if(serial_)
        if(serial_->scancomplete)
            return open_;
    return false;
}

void ComBt::close()
{

}

bool ComBt::send(const QByteArray &data)
{
    if(serial_)
        serial_->writeBytes(data);
    return true;
}

void ComBt::handleReadyRead()
{

}
