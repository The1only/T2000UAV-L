#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QAbstractSocket>
#include <QDebug>
#include <QTimer>
#include <QPlainTextEdit>

// #define USE_BT_IMU

#if not defined(Q_OS_ANDROID) && not defined(Q_OS_IOS)
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

#include <QList>

#ifdef Q_OS_ANDROID
#define SCREEN MainWindow_port_vertical
#include "ui_mainwindow_port_vertical.h"

//#define SCREEN MainWindow_port_new
//#include "ui_mainwindow_port_new.h"

//#define SCREEN MainWindow_port_small
//#include "ui_mainwindow_port_small.h"
#include "lockhelper.h"
#endif

#ifdef Q_OS_IOS
#define SCREEN MainWindow_port_iPhone
#include "ui_mainwindow_port_iPhone.h"
#endif

#ifdef Q_OS_MAC
#include "ui_mainwindow_port_new.h"
#define SCREEN MainWindow_port_new
//#define SCREEN MainWindow_port_small
//#include "ui_mainwindow_port_small.h"
#endif

//#include "ui_mainwindow_phone.h"
//#include "ui_mainwindow_port_small.h"
//#include "ui_mainwindow_small.h"
//#include "ui_mainwindow.h"

#include "ekfNavINS.h"
#include "rotation_matrix.h"

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = 0, QPlainTextEdit *s=NULL, void(*)(void *, QByteArray) = NULL, void(*)(void *, bool use_imu) = NULL);
    ~MyTcpSocket();

    void readyWrite(char *data);
    void doConnect();
    void (*ret_imu)(void *, bool use_imu);
    void (*ret)(void *,QByteArray);
    void connected();
    void connectedIMU();
    void setbacklit();

#if not defined(Q_OS_ANDROID) && not defined(Q_OS_IOS)
    int com_setup(QSerialPort *com_port, QString sport);
    QSerialPort *port = NULL;
    QSerialPort *lidar = NULL;
    QList<QSerialPortInfo> serialport;
#endif

    QString sport = "";
    QPlainTextEdit *text = NULL;
    bool isconnected = false;

#ifdef Q_OS_ANDROID
    QJniObject *someJavaObject = nullptr;
    QJniObject *imuJavaObject  = nullptr;
#else
    //In file CApi.cpp
    typedef struct Callbacks
    {
        void * classPtr;
        void(*callback)(void *);

    }Callbacks;

    //can be inited in some method. Must also be released somewhere. Or can be used with shared_ptr
    Callbacks * callbacks;

    void CallSwiftMemberFromC(void * classPtr, void(*callback)(void *)){
        callbacks->classPtr = classPtr;
        callbacks->callback = callback;

        std::function<void()> actaulCallback = [&](){
            callbacks->callback(callbacks->classPtr);
        };
        actaulCallback();
    }
#endif

    QString imuData = "";
#ifdef Q_OS_MAC
    QString Transponderstat = "true";
#else
    QString Transponderstat = "false";
#endif

    bool IMUconnected = false;
    bool m_external = true;
    bool m_imu_setup_done = false;

    /*  Return IMU elements....*/
    QString FromID = "";
    double AccX = 0.0;
    double AccY = 0.0;
    double AccZ = 0.0;
    double AsX = 0.0;
    double AsY = 0.0;
    double AsZ = 0.0;
    double AngleX = 0.0;
    double AngleY = 0.0;
    double AngleZ = 0.0;
    double HX = 0.0;
    double HY = 0.0;
    double HZ = 0.0;
    double Electricity = 0.0;
    double Temperature = 0.0;

signals:
    void sendMessage(const QString &message);

public slots:
    void doAlt();
    void doIMU();
    void doStart();
    void showMessage(const QString &sender, const QString &message) const;
    //   void processClientRequest();

private:
    QTimer* timer;
    QTimer* timerAlt;
    QTimer* timerIMU;
    QTimer* java;
    QTimer* timerStart;
    QObject *parent;

    int adapterFromUserSelection() const;
    int currentAdapterIndex = 0;

    void reactOnSocketError(const QString &error);

    QString localName;

};

#endif // MYTCPSOCKET_H
