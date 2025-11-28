#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QTcpSocket::connected,
            this, &TcpClient::onConnected);

    connect(&m_socket, &QTcpSocket::disconnected,
            this, &TcpClient::onDisconnected);

    connect(&m_socket, &QTcpSocket::readyRead,
            this, &TcpClient::onReadyRead);

    connect(&m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this,
            &TcpClient::onErrorOccurred);
}

void TcpClient::connectTo(const QHostAddress &address, quint16 port)
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        m_socket.abort();
    }
    m_socket.connectToHost(address, port);
}

void TcpClient::disconnectFrom()
{
    m_socket.disconnectFromHost();
}

void TcpClient::sendData(const QByteArray &data)
{
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        m_socket.write(data);
        m_socket.flush();
    }
}

void TcpClient::onConnected()
{
    emit connected();
}

void TcpClient::onDisconnected()
{
    emit disconnected();
}

void TcpClient::onReadyRead()
{
    QByteArray data = m_socket.readAll();
    emit dataReceived(data);
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError)
{
    emit errorOccurred(m_socket.errorString());
}
