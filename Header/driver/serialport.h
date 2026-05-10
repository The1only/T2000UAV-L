#ifndef COMQT_H
#define COMQT_H

#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <cstdint>
#include <functional>

#ifndef Q_OS_IOS
#include <QtSerialPort/QSerialPort>
#endif

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
#endif

/**
 * @brief Thin wrapper around QSerialPort used by MyTcpSocket.
 *
 * Responsibilities:
 *  - Open/close a serial port given a port name and baudrate.
 *  - Provide simple send() helpers for QByteArray / C-style data.
 *  - Emit Qt signals when data is received, connection changes, or errors occur.
 *  - Optionally forward incoming bytes to a C-style callback (used by C code).
 *
 * This class is intentionally minimal and is used as the "serial layer"
 * in the rest of the project (transponder, radar, INS).
 */
class ComQt : public QObject
{
    Q_OBJECT

public:
    explicit ComQt(QObject *parent = nullptr);
    ~ComQt();

    /**
     * @brief Open a serial port.
     *
     * @param portName OS-specific port identifier
     *        e.g. "COM4" on Windows, "/dev/tty.usbserial-14130" on macOS,
     *             "/dev/ttyUSB0" on Linux, or an Android-specific name.
     * @param baudrate Standard QSerialPort baud rate value
     *        (e.g. QSerialPort::Baud9600, QSerialPort::Baud115200).
     * @return true on success, false on failure.
     */
    bool open(const QString &portName, qint32 baudrate);

    /**
     * @brief Close the serial port if open.
     */
    void close();

    /**
     * @brief Change the baudrate of an already-open port.
     * @return true on success, false if port is closed or setting fails.
     */
    bool setBaudrate(qint32 baudrate);

    /**
     * @brief Send a QByteArray over the serial port.
     * @return true if all bytes were queued successfully.
     */
    bool send(const QByteArray &data);

    /**
     * @brief Send raw bytes over the serial port.
     *
     * @param data Pointer to data buffer.
     * @param len  Number of bytes to send.
     * @return true if all bytes were queued successfully.
     */
    bool send(const char *data, unsigned short len);

#ifndef Q_OS_IOS
    /// Underlying QSerialPort instance (nullptr on iOS).
    QSerialPort *serial_ = nullptr;

    /// Convenience wrapper: check if serial port is open.
    bool isOpen() const { return serial_ && serial_->isOpen(); }
#endif

signals:
    /**
     * @brief Emitted whenever the connection state changes.
     * @param connected true if port is now open, false if it was closed or failed.
     */
    void connectionChanged(bool connected);

    /**
     * @brief Emitted when new data are available on the serial port.
     */
    void dataReceived(const QByteArray &data);

    /**
     * @brief Emitted when an error occurs on the serial port.
     */
    void errorReceived(const QString &message);

public:
    /// Optional human-readable status string (const, set in ctor).
    const QString status;

    // ------------------------------------------------------------------
    // C-style RX callback support
    // ------------------------------------------------------------------

    /**
     * @brief Function pointer type for C-style RX callbacks.
     *
     * Signature must match exactly between typedef and stored std::function.
     */
    using RxCallback = void(*)(void *handler, const char *data, uint32_t length);

    /**
     * @brief Callback invoked when bytes arrive (if set).
     *
     * The first argument is a user-provided context pointer (typically
     * a "parent" object that will handle the data).
     *
     * NOTE: By convention in this project, the context pointer is passed in
     *       via MyTcpSocket when setRxCallback() is called.
     */
    std::function<void(void *, const char *, uint32_t)> callback_;

    /**
     * @brief Register a C-style RX callback.
     *
     * Only the function pointer is stored here; the user data / handler
     * pointer is provided later when handleReadyRead() is invoked.
     */
    void setRxCallback(RxCallback cb) { callback_ = cb; }

    /// Optional owner pointer (legacy; QObject::parent() is usually preferred).
    QObject *parent = nullptr;

#ifdef Q_OS_ANDROID
    /// Optional JNI helper object for Android-specific serial handling.
    QJniObject *someJavaObject = nullptr;
    QJniObject *imuJavaObject  = nullptr;
#endif

private:
    /// True while the port is considered "running" (open and active).
    bool running = false;

    /// Simple global pointer hook (legacy singleton-style usage, if needed).
    ComQt *g_comqt = nullptr;

private slots:
    /**
     * @brief Slot connected to QSerialPort::readyRead().
     *
     * Reads available data, emits dataReceived(), and forwards to callback_
     * if one has been registered with setRxCallback().
     */
    void handleReadyRead();
};

#endif // COMQT_H
