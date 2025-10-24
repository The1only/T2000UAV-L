#ifndef COMQT_H
#define COMQT_H

#include <QObject>
#ifndef Q_OS_IOS
#include <QtSerialPort/QSerialPort>
#endif
#include <cstdint>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
#endif

//#pragma once
#include <QByteArray>


class ComQt : public QObject
{
    Q_OBJECT
public:
    explicit ComQt(QObject *parent = nullptr);
    ~ComQt();

    // Open by port name (e.g. "COM4" on Windows, "/dev/tty.usbserial-14130" on macOS)
    bool open(const QString &portName, qint32 baudrate);

    void close();

    bool setBaudrate(qint32 baudrate);

    bool send(const QByteArray &data);
    bool send(const char *data, unsigned short len);

#ifndef Q_OS_IOS
    QSerialPort *serial_ = nullptr;
    bool isOpen() const { return serial_->isOpen(); }
#endif


signals:
    void connectionChanged(bool connected);          // <— add this
    void dataReceived(const QByteArray& data);       // <— if you emit it
    void errorReceived(const QString& message);      // <— if you emit it

public:
    const QString status;
    // If you use a C-style callback:
    // EXACT same signature for both typedef and std::function
    using RxCallback = void(*)(void* handler, const char* data, uint32_t length);

    std::function<void(void *, const char*, uint32_t)> callback_;
    void setRxCallback(RxCallback cb) { callback_ = cb; }

    QObject* parent;

#ifdef Q_OS_ANDROID
    QJniObject *someJavaObject = nullptr;
    QJniObject *imuJavaObject  = nullptr;
#endif

private:
    bool running;
    ComQt* g_comqt = nullptr;   // simple singleton-style forward — adjust to your needs


private slots:
    void handleReadyRead();
};

#endif // COMQT_H
