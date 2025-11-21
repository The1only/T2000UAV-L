#include "serialport.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>

ComQt::ComQt(QObject *Parent) : QObject(Parent)
{
    this->parent = Parent;
    serial_=new QSerialPort(this);
    connect(serial_, &QSerialPort::readyRead,
            this, &ComQt::handleReadyRead);
}

ComQt::~ComQt()
{
    close();
}

bool ComQt::open(const QString &portName, qint32 baudrate)
{
    if (serial_->isOpen())
        serial_->close();

    serial_->setPortName(portName);
    serial_->setBaudRate(baudrate);
    serial_->setDataBits(QSerialPort::Data8);
    serial_->setParity(QSerialPort::NoParity);
    serial_->setStopBits(QSerialPort::OneStop);
    serial_->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_->open(QIODevice::ReadWrite))
    {
        qWarning() << "Failed to open" << portName << ":" << serial_->errorString();
        return false;
    }
    serial_->flush();
    return true;
}

void ComQt::close()
{
    if (serial_->isOpen())
    {
        serial_->flush();
        serial_->close();
    }
}

bool ComQt::setBaudrate(qint32 baudrate)
{
    if(!serial_) return false;
    if (!serial_->isOpen()) return false;
    serial_->flush();
    return serial_->setBaudRate(baudrate);
}

bool ComQt::send(const QByteArray &data)
{
    if(!serial_) return false;
    if (!serial_->isOpen()) return false;
    qint64 written = serial_->write(data);
    if (written < 0) return false;
    return serial_->waitForBytesWritten(1000);
}

bool ComQt::send(const char *data, unsigned short len)
{
    return send(QByteArray::fromRawData(data, len));
}

void ComQt::handleReadyRead()
{
    QByteArray data = serial_->readAll();
    if (!data.isEmpty() && callback_)
    {
        char *dta = data.data();
        callback_(this->parent,dta,data.length()); //.data(), static_cast<uint32_t>(data.size()));

        // And emit Qt signal for other consumers
        emit dataReceived(data);
    }
}
