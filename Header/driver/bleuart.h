#ifndef BLEUART_H
#define BLEUART_H

#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothUuid>
#include <QSet>  // <-- add this
#include <QObject>
#include <QByteArray>

//static const QBluetoothUuid kNusService("{6E400001-B5A3-F393-E0A9-E50E24DCCA9E}");
//static const QBluetoothUuid kNusRx("{6E400002-B5A3-F393-E0A9-E50E24DCCA9E}"); // Write
//static const QBluetoothUuid kNusTx("{6E400003-B5A3-F393-E0A9-E50E24DCCA9E}"); // Notify


static const QBluetoothUuid wt901Service("{0000ffe5-0000-1000-8000-00805f9a34fb}");
static const QBluetoothUuid wt901Rx("{0000ffe9-0000-1000-8000-00805f9a34fb}"); // Write
static const QBluetoothUuid wt901Tx("{0000ffe4-0000-1000-8000-00805f9a34fb}"); // Notify
// f00e1ecb-31b9-c6e7-9633-63d3ea57e7e0

class BleUart : public QObject {
    Q_OBJECT
public:

    bool scancomplete = false;

    void startScan(int timeoutMs = 5000, int rssiMin = -95) {
        scancomplete = false;

        qDebug() << "Scanning BLE (no uuid filter in Qt6 dev-scan) for up to" << timeoutMs << "ms…";
        candidates.clear();
        seen.clear();
        targetSvc = QBluetoothUuid();
        if (agent) { agent->deleteLater(); agent = nullptr; }

        agent = new QBluetoothDeviceDiscoveryAgent(this);
        agent->setLowEnergyDiscoveryTimeout(timeoutMs);
        connect(agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,this, &BleUart::onDevice);
        connect(agent, &QBluetoothDeviceDiscoveryAgent::finished,this, &BleUart::onScanDone);

        minRssi = rssiMin;
        agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    }

    void connectTo(const QBluetoothDeviceInfo &dev) {
        ctl = QLowEnergyController::createCentral(dev, this);
        connect(ctl, &QLowEnergyController::connected, this, &BleUart::onConnected);
        connect(ctl, &QLowEnergyController::disconnected, this, &BleUart::onDisconnected);
        connect(ctl, &QLowEnergyController::serviceDiscovered, this, &BleUart::onServiceFound);
        connect(ctl, &QLowEnergyController::discoveryFinished, this, &BleUart::onServiceScanDone);
        connect(ctl, &QLowEnergyController::mtuChanged, this, [this](int mtu){
            payloadBytes = qMax(20, mtu - 3); // ATT header = 3 bytes
        });
        ctl->connectToDevice();
    }

    void writeBytes(const QByteArray &data) {
        if (!svc || !rx.isValid()) return;
        for (int i = 0; i < data.size(); i += payloadBytes) {
            const int n = qMin(payloadBytes, data.size() - i);
            svc->writeCharacteristic(rx, data.mid(i, n),
                                     QLowEnergyService::WriteWithoutResponse);
        }
    }

signals:
    void bytesReceived(QByteArray);
    void ready();                 // emitted when NUS service is discovered & CCCD enabled
    void disconnected();          // emitted when link drops
    void dataReceived(QByteArray);

private slots:
private slots:
    void onDevice(const QBluetoothDeviceInfo &d) {
        if (!(d.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
            return;

        // 1) Skip "no ID" devices (no name shown by the OS/stack)
        const QString name = d.name().trimmed();
        if (name.isEmpty())
            return;

        // 2) De-duplicate by deviceUuid (macOS reports 00:00:00:00:00:00 for address)
        const QUuid id = d.deviceUuid();
        if (seen.contains(id))
            return;

        // Optional: quick RSSI gate
   //     if (d.rssi() != 0 && d.rssi() < minRssi) return;

        // 3) Keep only NUS advertisers OR specific whitelisted names
        const auto advUuids = d.serviceUuids();
        const bool hasNus   = advUuids.contains(wt901Service);
//        const bool hasNus   = advUuids.contains(kNusService);
        static const QStringList targets = { "ESP32-NUS", "T2000-transponder", "WT901BLE67" };
        qDebug() << ">>>>>>>> Found:" << name << d.address().toString() << d.deviceUuid().toString();

        if (hasNus || targets.contains(name)) {
            seen.insert(id);
            qDebug() << ">>>>>>>> Target:" << name << d.address().toString() << d.deviceUuid().toString();
            candidates << d;

            //stop discovery; this will cause QBluetoothDeviceDiscoveryAgent::finished()
            // to be emitted (so onScanDone() will run).
            // 1) Prefer abort() for immediate, forceful stop:
            if (agent && agent->isActive()) {
                agent->finished();   // usually immediate
                qDebug() << "The scan should stop now...";
            }
        } else {
            // Not a match → ignore
            // qDebug() << "Ignoring:" << name;
        }
    }

    void onScanDone() {
        agent->stop();
        qDebug() << "Scan complete. Found" << candidates.size() << "targets.";
        scancomplete = true;
//        if(candidates.length() == 0) scancomplete = true;
        for (const auto &dev : candidates){
            connectTo(dev);   // optionally connect to each
        }
    }
    void onConnected() {
        ctl->discoverServices();
    }
    void onServiceFound(const QBluetoothUuid &uuid) {
        if (uuid == wt901Service) targetSvc = uuid;
//        if (uuid == kNusService) targetSvc = uuid;
    }
    void onServiceScanDone() {
        if (targetSvc.isNull()) {
            // Verify NUS after connect → discoverServices() has finished
            if (ctl->services().contains(wt901Service)) targetSvc = wt901Service;
        }
        if (!targetSvc.isNull()) {
            svc = ctl->createServiceObject(targetSvc, this);
            if (!svc) return;
            connect(svc, &QLowEnergyService::stateChanged, this, &BleUart::onSvcState);
            connect(svc, &QLowEnergyService::characteristicChanged, this,
                    [this](const QLowEnergyCharacteristic &c, const QByteArray &v){
                        if (c.uuid() == wt901Tx) emit bytesReceived(v);
                    });
            svc->discoverDetails();
        } else {
            qDebug() << "Connected device has no NUS → disconnecting";
            ctl->disconnectFromDevice();
        }
    }
    void onSvcState(QLowEnergyService::ServiceState s) {
        if (s != QLowEnergyService::ServiceDiscovered) return;
        rx = svc->characteristic(wt901Rx);
        tx = svc->characteristic(wt901Tx);

        // Enable notifications (Qt 6 enum location)
        auto cccd = tx.descriptor(QBluetoothUuid(
            QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration));
        if (cccd.isValid())
            svc->writeDescriptor(cccd, QByteArray::fromHex("0100"));

        emit ready();                 // <— tell ComBt we’re good to send

    }
    void onDisconnected() {
        emit disconnected();          // <— tell ComBt we’re not connected anymore
    }

public:
    using RxCallback = void(*)(void *handler, const char* data, uint32_t length); //char*, uint32_t);
    void setRxCallback(RxCallback cb) { callback_ = cb; }

private:
//    RxCallback  callback_ = nullptr;
    std::function<void(void *, const char*, uint32_t)> callback_;


    QBluetoothDeviceDiscoveryAgent *agent = nullptr;
    QList<QBluetoothDeviceInfo> candidates;
    QSet<QUuid> seen;                 // <-- add this to avoid duplicates
    QLowEnergyController *ctl = nullptr;
    QBluetoothUuid targetSvc;
    QLowEnergyService *svc = nullptr;
    QBluetoothDeviceInfo currentDev;  // optional if you want to track
    QLowEnergyCharacteristic rx, tx;
    int payloadBytes = 20;
    int minRssi = -95;
};

//class BleUart; // fwd

class ComBt : public QObject
{
    Q_OBJECT
public:
    explicit ComBt(QObject *parent = nullptr);
    ~ComBt();

    BleUart *serial_ = nullptr;

    // BLE “open” – kicks off scan/connect via BleUart, ignores portName/baudrate
    bool open(const QString &portName = QString(), qint32 baudrate = 0);
    void close();

    bool setBaudrate(qint32) { return true; }  // not applicable for BLE; always OK

    bool send(const QByteArray &data);
    bool send(const char *data, unsigned short len) { return send(QByteArray::fromRawData(data, len)); }

    bool scanDone = false;
    const QString status;
    // If you use a C-style callback:
    // EXACT same signature for both typedef and std::function
    using RxCallback = void(*)(void* handler, const char* data, uint32_t length);

    std::function<void(void *, const char*, uint32_t)> callback_;
    void setRxCallback(RxCallback cb) {
        serial_->setRxCallback(cb);
        callback_ = cb;
    }

    QObject* parent;

signals:
    void connectionChanged(bool connected);
    void dataReceived(const QByteArray &data);
    void errorReceived(const QString &message);

private:
    bool open_      = true;
    bool running;
    ComBt* g_comqt  = nullptr;   // simple singleton-style forward — adjust to your needs


private slots:
    void handleReadyRead();
};

#endif // BLEUART_H
