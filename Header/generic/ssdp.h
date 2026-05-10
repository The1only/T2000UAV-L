// SsdpDiscoverer.h
#pragma once

/**
 * @file ssdp.h
 * @brief SSDP (Simple Service Discovery Protocol) discovery support.
 *
 * Declares the SsdpDiscoverer class, which provides basic SSDP discovery
 * functionality using UDP multicast and integrates with Qt's event loop.
 *
 * This header is part of a larger Qt 6 project. Public API stability is
 * required; names and signatures must not change.
 */

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>

#define SENSOR       "IMU"          // IMU, RADAR, TRANSPONDER, etc.
#define SENSOR_TYPE  "Airplane-device"   // change per firmware type

static const QHostAddress SSDP_GROUP("255.255.255.255");
static const quint16 SSDP_PORT = 4210;

struct SensorService {
    const char *name;    // logical name, e.g. "IMU"
    const char *st;      // ST value, e.g. "imu-device"
    const char *uuid;    // per-service UUID
};

// Our custom service type
static const char *SSDP_ST = SENSOR_TYPE;

class SsdpDiscoverer : public QObject
{
    Q_OBJECT
public:
    explicit SsdpDiscoverer(QObject *parent = nullptr);

    // Start a discovery burst (one M-SEARCH + listen for replies for a few seconds)
    void startDiscovery(int durationMs = 2000);

signals:
    void deviceFound(const QHostAddress &addr,
                     quint16 port,
                     const QString &st);

private slots:
    void onReadyRead();

private:
    QUdpSocket m_socket;
    QTimer     m_timer;
};
