#ifndef BTCOMQT_H
#define BTCOMQT_H

#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <cstdint>
#include <QTimer>

class BTComQt : public QObject
{
    Q_OBJECT
public:
    explicit BTComQt(QObject *parent = nullptr);
    ~BTComQt();

    // Open by port name (e.g. "COM4" on Windows, "/dev/tty.usbserial-14130" on macOS)
    bool open(const QString &portName, qint32 baudrate);

    void close();

    bool send(const QByteArray &data);
    bool send(const char *data, unsigned short len);

    QJniObject *imuJavaObject  = nullptr;

    using RxCallback = void(*)(void *handler, const char* data, uint32_t length); //char*, uint32_t);
//    using RxCallback = void(*)(void *, const QByteArray &data); //char*, uint32_t);
    void setRxCallback(RxCallback cb) { callback_ = cb; }

//private:
    QSerialPort *serial_ = nullptr;
    QTimer*    timerAndroid;

private:
     RxCallback  callback_ = nullptr;
     QObject *parent;

signals:
    void dataReceived(const QByteArray &data);

private slots:
    void handleReadyRead();
};

#endif // BTCOMQT_H
