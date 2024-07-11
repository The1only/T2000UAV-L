#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDebug>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QSerialPort>
#include <QSerialPortInfo>
#endif
#include <QList>
#include <QtBluetooth/qbluetoothhostinfo.h>

#include "remoteselector.h"
#include "chatserver.h"
#include "chatclient.h"

#include "ui_mainwindow_port.h"
#include "ui_mainwindow.h"

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = 0, QPlainTextEdit *s=NULL, void(*)(QByteArray) = NULL, void(*)(QByteArray) = NULL);
    void readyWrite(char *data);
    void doConnect();
    void (*ret_lidar)(QByteArray);
    void (*ret)(QByteArray);
#ifdef Q_OS_ANDROID
    int com_setup(QSerialPort *com_port, QString sport);
    QSerialPort *port = NULL;
    QSerialPort *lidar = NULL;
    QList<QSerialPortInfo> serialport;
#endif
    QString sport = "";
    QPlainTextEdit *text = NULL;
    bool isconnected = false;

signals:
    void sendMessage(const QString &message);

public slots:
    void connected();
    void disconnected();
    void doWork();
    void doAlt();
    void showMessage(const QString &sender, const QString &message) const;

private:
    QTcpSocket *socket = NULL;
    QTimer* timer;
    QTimer* timerAlt;

    int adapterFromUserSelection() const;
    int currentAdapterIndex = 0;

    ChatServer *server;
    QList<QBluetoothHostInfo> localAdapters;
    QList<ChatClient *> clients;

    void reactOnSocketError(const QString &error);
    void clientDisconnected();


    QString localName;

};

#endif // MYTCPSOCKET_H
