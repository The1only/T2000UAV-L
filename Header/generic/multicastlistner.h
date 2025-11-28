#pragma once

#include <QObject>
#include <QHostAddress>

class QSocketNotifier;

class MulticastListener : public QObject
{
    Q_OBJECT
public:
    explicit MulticastListener(QObject *parent = nullptr);
    ~MulticastListener() override;

    bool start(const QString &groupAddr = QStringLiteral("239.255.0.1"),
               quint16 port = 4210);
    void stop();

signals:
    void messageReceived(const QString &message,
                         const QHostAddress &sender,
                         quint16 senderPort);
    void errorOccurred(const QString &error);

private slots:
    void onSocketActivated(int socket);

private:
    int             m_fd = -1;
    QSocketNotifier *m_notifier = nullptr;
    QString         m_group;
    quint16         m_port = 0;
};
