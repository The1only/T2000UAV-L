// mytcpsocket.cpp

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include "remoteselector.h"
#include "chatserver.h"
#include "chatclient.h"

#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothuuid.h>

#include "mytcpsocket.h"

//static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");
static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805f9b34fb");
//static const QLatin1String serviceUuid("00001124-0000-1000-8000-00805f9b34fb");
//static const QLatin1String serviceUuid(  "C586467F-B255-5170-2F3F-63FDA1AE3AF7");

#ifdef Q_OS_ANDROID
//static const QLatin1String reverseUuid("c8e96402-0102-cf9c-274b-701a950fe1e8");
static const QLatin1String reverseUuid("00001101-0000-1000-8000-00805f9b34fb");
//static const QLatin1String reverseUuid("bf43b9f5-0800-0008-0001-000010110000");
//static const QLatin1String reverseUuid("bf43b9f5-0800-0008-0001-000042110000");
//static const QLatin1String serviceUuid(  "7FA3EA1A-DF63-F32F-0715-552BF764685C");
#endif

MyTcpSocket::MyTcpSocket(QObject *parent,  QPlainTextEdit *s, void (*retx)(QByteArray)) :
    QObject(parent)
{
    this->ret = retx;
    this->text = s;

    localAdapters = QBluetoothLocalDevice::allDevices();

    // make discoverable
    if (!localAdapters.isEmpty()) {
        QBluetoothLocalDevice adapter(localAdapters.at(0).address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    } else {
        qWarning("Local adapter is not found! The application might work incorrectly.");
#ifdef Q_OS_WIN
        // WinRT implementation does not support adapter information yet. So it
        // will always return an empty list.
        qWarning("If the adapter exists, make sure to pair the devices manually before launching"
                 " the chat.");
#endif
    }

    //! [Create Chat Server]
    server = new ChatServer(this);
    connect(server, &ChatServer::messageReceived,this,&MyTcpSocket::showMessage);
    connect(this, &MyTcpSocket::sendMessage, server, &ChatServer::sendMessage);
    server->startServer();
    //! [Create Chat Server]

    //! [Get local device name]
    localName = QBluetoothLocalDevice().name();
    //! [Get local device name]


    this->serialport = QSerialPortInfo::availablePorts();
    for (QSerialPortInfo y : this->serialport)
    {
        QString n = y.portName();
        qDebug() << n;

      //  if(n.contains("tty"))
            this->text->appendPlainText(n);

        //        if(n.contains("tty.ESP32test") || n.contains("tty.usbserial-110"))
        if(n.contains("ttyUSB0") || n.contains("tty.usbserial-110"))
        {
          // if(n.contains("tty.usbserial-110"))
                sport = "/dev/"+n;
          // else
            //    sport = n;

            this->text->appendPlainText("Using this port: ");
            this->text->appendPlainText(sport);
        }
    }
}

void MyTcpSocket::showMessage(const QString &sender, const QString &message) const
{
    qDebug() << "From: " << sender << " received: " << message;
    QByteArray tmp = message.toUtf8();
    tmp.append('\0');
    this->ret(tmp);
}


void MyTcpSocket::doConnect()
{ 
    //use a timer to allow the constructor to exit
    timerAlt = new QTimer(this);
    timerAlt->setSingleShot(false);
    connect(timerAlt, SIGNAL(timeout()), this, SLOT(doAlt()));

    // scan for services
    const QBluetoothAddress adapter = localAdapters.isEmpty() ?
                                           QBluetoothAddress() :
                                           localAdapters.at(currentAdapterIndex).address();

    RemoteSelector remoteSelector(adapter);
#ifdef Q_OS_ANDROID
    // QTBUG-61392
    Q_UNUSED(serviceUuid);
    remoteSelector.startDiscovery(QBluetoothUuid(reverseUuid));
#else
    remoteSelector.startDiscovery(QBluetoothUuid(serviceUuid));
#endif

    // Do we use Bluetooth...?
    if (remoteSelector.exec() == QDialog::Accepted) {
        QBluetoothServiceInfo service = remoteSelector.service();

        qDebug() << "Connecting to service 2" << service.serviceName()
                 << "on" << service.device().name();

        // Create client
        qDebug() << "Going to create client";
        ChatClient *client = new ChatClient(this);

        qDebug() << "Connecting...";

        connect(client, &ChatClient::messageReceived,this, &MyTcpSocket::showMessage);
        connect(this, &MyTcpSocket::sendMessage, client, &ChatClient::sendMessage);

        connect(client, &ChatClient::disconnected, this, QOverload<>::of(&MyTcpSocket::clientDisconnected));
        connect(client, QOverload<const QString &>::of(&ChatClient::connected), this, &MyTcpSocket::connected);
        connect(client, &ChatClient::socketErrorOccurred,  this, &MyTcpSocket::reactOnSocketError);

        qDebug() << "Start client";
        client->startClient(service);

        clients.append(client);
    }

//! [Connect to remote service]

    // Do we use Serial port ?
    if(sport != "" && clients.length() == 0)
    {
        qDebug() << "Opening port: " << sport;
        port = new QSerialPort(sport);
        this->text->appendPlainText("Opening port...");

        port->setBaudRate(QSerialPort::Baud9600);
        port->setDataBits(QSerialPort::Data8);
        port->setStopBits(QSerialPort::OneStop);
        port->setParity(QSerialPort::NoParity);
        port->setFlowControl(QSerialPort::NoFlowControl);

        QObject::connect(port, &QSerialPort::bytesWritten, [](qint64 bytes){ qDebug() << bytes << " bytes written...";});
        QObject::connect(port, &QSerialPort::readyRead, [this](){ this->ret(this->port->readAll());});
        QObject::connect(port, &QSerialPort::errorOccurred, [](QSerialPort::SerialPortError error){ qDebug() << error; });

        if (port->open(QSerialPort::OpenModeFlag::ReadWrite))
        {
            this->text->appendPlainText("Port open...");
            timerAlt->start(500);
            this->isconnected = true;
        }
        qDebug() <<  port->error();
        this->text->appendPlainText(port->errorString());
    }

    // Do we used UDP...?
    if( this->isconnected == false && clients.length() == 0){
        socket = new QTcpSocket(this);
        QObject::connect(socket, SIGNAL(connected()),this, SLOT(connected()));
        QObject::connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));

        QObject::connect(socket, &QTcpSocket::bytesWritten, [](qint64 bytes){ qDebug() << bytes << " bytes written...";});
        QObject::connect(socket, &QTcpSocket::readyRead, [this](){ this->ret(this->socket->readAll());});
        QObject::connect(socket, &QTcpSocket::errorOccurred, [](QTcpSocket::SocketError error){ qDebug() << error; });

        //use a timer to allow the constructor to exit
        timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->start(100);
        connect(timer, SIGNAL(timeout()), this, SLOT(doWork()));
    }
}

void MyTcpSocket::reactOnSocketError(const QString &error)
{
     qDebug() << error;
}

void MyTcpSocket::clientDisconnected()
{
    ChatClient *client = qobject_cast<ChatClient *>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
    }
}

void MyTcpSocket::doAlt()
{
    static int state=0;

    if(this->isconnected == true){
        switch(state){
        case 0:
            this->readyWrite("v=1\r\n");
            break;
        case 1:
            this->readyWrite("z=?\r\n");
            break;
        case 2:
            this->readyWrite("a=?\r\n");
            break;
        case 3:
            this->readyWrite("c=?\r\n");
            break;
        case 4:
            this->readyWrite("s=?\r\n");
            break;
        case 5:
            this->readyWrite("i=?\r\n");
            break;
        case 6:
            this->readyWrite("r=?\r\n");
            break;
        }

        if(++state > 6){
            state = 2;
        }
    }
//    timerAlt->start(250);
}

void MyTcpSocket::doWork()
{
    qDebug() << "connecting...";

    // this is not blocking call
    socket->connectToHost("0.0.0.0", 6000);

    // we need to wait...
    if(!socket->waitForConnected(50000))
    {
        qDebug() << "Error: " << socket->errorString();
        timer->start(1500);
    }
}

void MyTcpSocket::connected()
{
    this->isconnected = true;    
    qDebug() << "connected...";
    timerAlt->start(500);

}

void MyTcpSocket::disconnected()
{
    this->isconnected = false;
    qDebug() << "disconnected...";
    timerAlt->stop();
    this->doConnect();
}

void MyTcpSocket::readyWrite(char *data)
{
    if(this->isconnected == true){

        if(socket)
        {
            if(this->isconnected == true)
            {
                socket->write(data);
                this->text->appendPlainText(data);
            }
        }

        if(port){
            if(port->isWritable()){
                port->write(data);
                this->text->appendPlainText(data);
            }
        }

        emit sendMessage(data);
    }
}
