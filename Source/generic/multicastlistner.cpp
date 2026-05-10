#include "multicastlistner.h"

#include <QSocketNotifier>
#include <QDebug>

// POSIX / BSD sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>   // close()

MulticastListener::MulticastListener(QObject *parent)
    : QObject(parent)
{
}

MulticastListener::~MulticastListener()
{
    stop();
}

bool MulticastListener::start(const QString &groupAddr, quint16 port)
{
    stop();

    m_group = groupAddr;
    m_port  = port;

    // 1. Create UDP socket
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_fd < 0) {
        emit errorOccurred(QStringLiteral("socket() failed"));
        return false;
    }

    // 2. Allow multiple listeners on same port (like Python SO_REUSEADDR)
    int yes = 1;
    if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        emit errorOccurred(QStringLiteral("setsockopt(SO_REUSEADDR) failed"));
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

#ifdef SO_REUSEPORT
    // Often needed on macOS for multicast sharing
    ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif

    // 3. Bind to INADDR_ANY:port  (same as Python's sock.bind(('', port)))
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(m_port);

    if (::bind(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        emit errorOccurred(QStringLiteral("bind() failed on port %1").arg(m_port));
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    // 4. Join multicast group (like Python's IP_ADD_MEMBERSHIP)
    ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr = ::inet_addr(m_group.toUtf8().constData());
    // Use INADDR_ANY to let OS pick the right interface (Wi-Fi), just like Python
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (::setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     &mreq, sizeof(mreq)) < 0) {
        emit errorOccurred(QStringLiteral("setsockopt(IP_ADD_MEMBERSHIP) failed"));
        ::close(m_fd);
        m_fd = -1;
        return false;
    }

    // 5. Hook into Qt event loop
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, &MulticastListener::onSocketActivated);

    qDebug() << "Listening for multicast on" << m_group << ":" << m_port;
    return true;
}

void MulticastListener::stop()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }

    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

void MulticastListener::onSocketActivated(int)
{
    // Read all pending datagrams
    while (true) {
        sockaddr_in senderAddr{};
        socklen_t senderLen = sizeof(senderAddr);
        char buf[2048];

        ssize_t n = ::recvfrom(m_fd, buf, sizeof(buf) - 1, 0,
                               reinterpret_cast<sockaddr *>(&senderAddr),
                               &senderLen);
        if (n < 0) {
            // EWOULDBLOCK etc.
            break;
        }

        buf[n] = '\0'; // ensure null-terminated for QString
        char senderIp[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &senderAddr.sin_addr,
                    senderIp, sizeof(senderIp));
        quint16 senderPort = ntohs(senderAddr.sin_port);

        QString msg = QString::fromUtf8(buf).trimmed();
        QHostAddress senderAddress(QString::fromLatin1(senderIp));

        emit messageReceived(msg, senderAddress, senderPort);
    }
}
