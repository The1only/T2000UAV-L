// SsdpDiscoverer.cpp
#include "ssdp.h"
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QVariant>
#include <unistd.h>   // usleep

SsdpDiscoverer::SsdpDiscoverer(QObject *parent)
    : QObject(parent)
{
        if (!m_socket.bind(QHostAddress::AnyIPv4,
                           SSDP_PORT,
                           QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
            qWarning() << "bind failed:" << m_socket.errorString();
            return;
        }

        // Enable receiving broadcast (defensive, often not strictly needed)
        m_socket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 256*1024);

        connect(&m_socket, &QUdpSocket::readyRead,
                this, &SsdpDiscoverer::onReadyRead);

 //       m_timer.setSingleShot(true);
 //       connect(&m_timer, &QTimer::timeout,
 //               this, &SsdpDiscoverer::onDiscoveryTimeout);

}

// Must for some reason be run....
void SsdpDiscoverer::startDiscovery(int durationMs)
{
    // SSDP M-SEARCH request
    QByteArray req;
    req.append("M-SEARCH * HTTP/1.1\r\n");
    req.append("HOST: ");
    req.append(QByteArray::fromStdString(SSDP_GROUP.toString().toStdString()));
    req.append(":");
    req.append(QByteArray::number(SSDP_PORT));
    req.append("\r\nMAN: \"ssdp:discover\"\r\n");
    req.append("MX: 3\r\n");                     // wait up to 1s for replies
    req.append("ST: ");
    req.append(SSDP_ST);                         // our custom type
    req.append("\r\n");
    req.append("\r\n");

    qint64 written = m_socket.writeDatagram(req, SSDP_GROUP, SSDP_PORT);
    if (written == -1) {
        qWarning() << "Failed to send SSDP M-SEARCH:"
                   << m_socket.error()
                   << m_socket.errorString();
    } else {
      //  qDebug() << "Sent SSDP M-SEARCH for" << SSDP_ST;
    }
  //  qDebug() << req;

 //   m_timer.start(durationMs); // keep listening for replies
}

void SsdpDiscoverer::onReadyRead()
{
    while (m_socket.hasPendingDatagrams()) {
        QHostAddress sender;
        quint16 senderPort = 0;

        QByteArray datagram;
        datagram.resize(int(m_socket.pendingDatagramSize()));
        qint64 len = m_socket.readDatagram(datagram.data(),
                                           datagram.size(),
                                           &sender,
                                           &senderPort);
        if (len <= 0)
            return;

        datagram.truncate(int(len));
        const QString text = QString::fromUtf8(datagram);
/*
        qDebug() << "SSDP response from" << sender.toString()
                 << ":" << senderPort << "\n"
                 << text;
*(
        /*
        const QStringList lines = text.split(" ", Qt::SkipEmptyParts);
        if(lines.length() ==2){
        QString st = lines[0];
            emit deviceFound(sender, senderPort, st);
        }
        */

        // Very simple parsing: look for "ST:", "USN:", "LOCATION:"
        QString st, usn, location;
        const QStringList lines = text.split("\r\n", Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            const QString lower = line.toLower();
            if (lower.startsWith("st:")) {
                st = line.mid(3).trimmed();
            } else if (lower.startsWith("usn:")) {
                usn = line.mid(4).trimmed();
            } else if (lower.startsWith("location:")) {
                location = line.mid(9).trimmed();
            }
        }

        if (st.compare(QLatin1String(SSDP_ST), Qt::CaseInsensitive) == 0) {
            emit deviceFound(sender, senderPort, usn);
        }
    }
}

void SsdpDiscoverer::onDiscoveryTimeout()
{
    qDebug() << "SSDP discovery timeout.";
    // Optional: emit a signal like discoveryFinished() if you want
}
