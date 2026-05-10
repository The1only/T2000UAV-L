#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>


class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

    // Call this with the ESP32 IP and port (23 in your case)
    void connectTo(const QHostAddress &address, quint16 port = 23);
    void disconnectFrom();

    // Sends raw bytes (for telnet-style text, you can append "\r\n")
    void sendData(const QByteArray &data);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);
    void dataReceived(const QByteArray &data);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket m_socket;
};
