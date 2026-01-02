/**
 * @file ssdp.cpp
 * @brief Implementation of SSDP discovery support.
 *
 * Contains the internal implementation details of SsdpDiscoverer,
 * including socket setup, multicast handling, and datagram parsing.
 */

#include "ssdp.h"
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QVariant>
#include <unistd.h>   // usleep

/**
 * @brief SSDP (Simple Service Discovery Protocol) helper class.
 *
 * This class provides basic SSDP discovery support using UDP multicast.
 * It can listen for SSDP NOTIFY messages and process incoming discovery
 * traffic.
 *
 * Responsibilities:
 * - Managing a UDP socket bound to the SSDP multicast group
 * - Receiving and parsing SSDP datagrams
 *
 * Non-responsibilities:
 * - Parsing device description XML
 * - Managing device lifetimes
 * - Thread synchronization
 *
 * Threading:
 * - Must be used from the thread it is created in
 *
 * Typical usage:
 * - Create an instance
 * - Start listening for SSDP traffic
 * - React to parsed SSDP messages
 */
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
}

/**
 * @brief Sends an SSDP M-SEARCH discovery request.
 *
 * Constructs and transmits a single SSDP M-SEARCH request to the
 * standard SSDP multicast group in order to actively discover
 * compatible devices or services on the local network.
 *
 * The request uses the following SSDP headers:
 * - @c HOST : SSDP multicast address and port
 * - @c MAN  : "ssdp:discover" (required for discovery requests)
 * - @c MX   : Maximum wait time (in seconds) for responses
 * - @c ST   : Search target (custom service type)
 *
 * Responses, if any, are delivered asynchronously via the UDP socket
 * and processed elsewhere in the class.
 *
 * @param durationMs Requested discovery duration in milliseconds.
 *        Currently unused by this implementation; discovery consists
 *        of sending a single M-SEARCH request.
 *
 * @note This function does not block and does not wait for responses.
 * @note A failure to send the datagram is reported via qWarning().
 *
 * @sa onReadyRead()
 */
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
    }
}

/**
 * @internal
 * @brief Processes incoming SSDP UDP datagrams.
 *
 * This slot is invoked when the underlying UDP socket reports that
 * one or more datagrams are available for reading. All pending
 * datagrams are drained from the socket and processed sequentially.
 *
 * Each datagram is interpreted as a UTF-8 encoded SSDP message and
 * parsed using a minimal, line-based approach. The following SSDP
 * headers are extracted if present:
 * - @c ST       (Search Target)
 * - @c USN      (Unique Service Name)
 * - @c LOCATION (Device description URL)
 *
 * Parsing is intentionally simple and tolerant; headers are matched
 * case-insensitively and no validation of message structure is
 * performed beyond basic prefix checks.
 *
 * If the received @c ST header matches the configured search target,
 * the @c deviceFound() signal is emitted.
 *
 * @note This function performs no blocking operations and runs in the
 *       thread owning the QUdpSocket.
 * @note A datagram read error causes immediate return and stops
 *       processing of any remaining pending datagrams.
 *
 * @sa startDiscovery()
 * @sa deviceFound()
 */
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

