#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QAbstractSocket>
#include <QDebug>
#include <QTimer>
#include <QPlainTextEdit>
#include <QDialog>
//#include <QtQuickWidgets/QQuickWidget>

// Look for an external IMU...
#define USE_BT_IMU

#ifdef Q_OS_IOS
#undef Q_OS_MAC
#endif

#if not defined(Q_OS_ANDROID) && not defined(Q_OS_IOS)
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

#include <QList>
//#include "ui_mainwindow_phone.h"
//#include "ui_mainwindow_port_small.h"
//#include "ui_mainwindow_small.h"
//#include "ui_mainwindow.h"

#include "ekfNavINS.h"
#include "rotation_matrix.h"
#include "serialport.h"
#include "BleUart.h"


#ifdef Q_OS_ANDROID
//    #include "serialport_bluetooth.h"

//    #define SCREEN MainWindow_port_vertical
//    #include "ui_mainwindow_port_vertical.h"

    #define SCREEN MainWindow_port_new
    #include "ui_mainwindow_port_new.h"

//   #define SCREEN MainWindow_port_iPhone
//   #include "ui_mainwindow_port_iPhone.h"

//  #define SCREEN MainWindow_port_small
//  #include "ui_mainwindow_port_small.h"

    #include "lockhelper.h"

    // Do we whant to simulate the GPS using a gpx file?
    #define simGPS false

#else
    #ifdef Q_OS_IOS
        #define SCREEN MainWindow_port_iPhone
        #include "ui_mainwindow_port_iPhone.h"
        #define simGPS false
    #else
        #include "ui_mainwindow_port_new.h"
        #define SCREEN MainWindow_port_new
        //#define SCREEN MainWindow_port_small
        //#include "ui_mainwindow_port_small.h"
        #define simGPS true
    #endif
#endif

static constexpr char STX = 0x02;
static constexpr char ETX = 0x03;
/*
class NoButtonMessageBox : public QDialog {
public:
    NoButtonMessageBox(const QString &message, QWidget *parent = nullptr)
        : QDialog(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        setModal(true);

        auto layout = new QVBoxLayout(this);
        QLabel *label = new QLabel(message, this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("QLabel { font-size: 18pt; color: white; background-color: #333; padding: 20px; border-radius: 12px; }");
        layout->addWidget(label);
        setLayout(layout);
        resize(300, 100);
    }
};
*/

class NoButtonMessageBox : public QDialog {
    Q_OBJECT
public:
    explicit NoButtonMessageBox(const QString &message, QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        setModal(true);

        auto layout = new QVBoxLayout(this);
        m_label = new QLabel(message, this);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setStyleSheet(
            "QLabel { "
            "font-size: 18pt; "
            "color: white; "
            "background-color: #333; "
            "padding: 20px; "
            "border-radius: 12px; }"
            );
        layout->addWidget(m_label);
        setLayout(layout);
        resize(300, 100);
    }

    // 🔹 Add this function
    void setText(const QString &text)
    {
        if (m_label)
            m_label->setText(text);
    }

private:
    QLabel *m_label = nullptr;
};

// A convenient record for each port (optional but useful)
struct PortEntry {
    QString serial;          // e.g. "4150323833373205"
    QString portName;        // e.g. "cu.usbmodem1301" or "COM5"
    QString systemLocation;  // e.g. "/dev/cu.usbmodem1301"
    QString description;     // e.g. "XIAO RP2040"
    QString manufacturer;    // e.g. "Seeed"
    quint16 vendorId = 0;    // optional
    quint16 productId = 0;   // optional
};

class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = 0, QPlainTextEdit *s=NULL, void(*)(void *, const char*, uint32_t) = NULL, void(*)(void *, bool use_imu) = NULL);
    ~MyTcpSocket();

    void readyWrite(char *data);
    void doConnect();
    void (*ret_imu)(void *, bool use_imu);
    void (*ret)(void *, const char *data, uint32_t size); //const QByteArray &data);
    void connected();
    void connectedIMU();
    void setbacklit();
    void setSerialPorts(QString imu, QString transponder, QString radar);


#ifdef Q_OS_MAC
    static QVector<PortEntry> listSerialPortsDetailed();
    static QMap<QString, QString> serialToPortMap(bool useSystemLocation = true);
    QString findPort(QString targetSerial);
    QMap<QString, QString> map;
#endif

#if not defined(Q_OS_ANDROID) && not defined(Q_OS_IOS)
    int com_setup(QSerialPort *com_port, QString sport);
    QSerialPort *port = NULL;
    QSerialPort *lidar = NULL;
    QList<QSerialPortInfo> serialport;
#endif

    QString sport = "";
    QPlainTextEdit *text = NULL;

    QString _transponder_id = "";
    QString _radar_id = "";
    QString _IMU_id = "";

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
    bool Transponderstat = true;
#else
    bool Transponderstat = false;
#endif

    bool Radarstat = false;
    float rPos  =0.0;
    float rSpeed=0.0;
    float rDist =0.0;

    bool IMUconnected = false;
    bool m_external = true;
    bool m_imu_setup_done = false;

    double m_preasure_QNH  = -10000;
    bool   TransponderstatWithBarometer = false;

    /*  Return IMU elements....*/
    QString FromID = "";
    double AccX = 0.0;
    double AccY = 0.0;
    double AccZ = 0.0;
    double G    = Gfix;
    double AsX = 0.0;
    double AsY = 0.0;
    double AsZ = 0.0;
    double AngleX = 0.0;
    double AngleY = 0.0;
    double AngleZ = 0.001;
    double HX = 0.0;
    double HY = 0.0;
    double HZ = 0.0;
    unsigned int VER = 0.0;
    double Temp = 0.0;
    double LAT = 0.0;
    double LON = 0.0;
    double ALT = 0.0;
    double AIR_PRESSURE = 0.0;
    double BARO_ALT = 0.0;
    double FW_Speed = 0.0;
    double Donwn_Speed = 0.0;

    int Orient = 0;

//#ifdef Q_OS_MAC

private:
    ComQt *TransponderSerPort= nullptr;
    ComQt *RadarSerPort = nullptr;
    ComQt *INSSerPort =  nullptr;
    ComBt *bluetootPort = nullptr;

signals:
    void sendMessage(const QString &message);

public slots:
    void doAlt();
    void doStart();

    static void doIMU(void *parent, const char *data, uint32_t length); //const QByteArray &data);
    static void doRadar(void *parent, const char *data, uint32_t length); //const QByteArray &data);
  //  void SensorUartSend(uint8_t *p_data, uint32_t uiSize);

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
