/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DEVICE_H
#define DEVICE_H

#include <qbluetoothlocaldevice.h>
#include <QObject>
#include <QVariant>
#include <QList>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QBluetoothServiceInfo>

#include "deviceinfo.h"

QT_FORWARD_DECLARE_CLASS (QBluetoothDeviceInfo)
QT_FORWARD_DECLARE_CLASS (QBluetoothServiceInfo)

class Device: public QObject
{
    Q_OBJECT

public:
    Device();
    ~Device();
    /*
    QVariant getDevices();
    QVariant getServices();
    QVariant getCharacteristics();
    QString getUpdate();
    */
    bool state();
    void SendData(QString data, int ch);

    QString m_message;
    QLowEnergyService *l_service = nullptr;
    QString key2;
    QString receivedData;


public slots:
    void startDeviceDiscovery();
    void scanServices(const QString &address);

    void connectToService(const QString &uuid);
    void disconnectFromDevice();
    void setVal();
    void test();
    void updateValueSet();


private slots:
    // QBluetoothDeviceDiscoveryAgent related
    void addDevice(const QBluetoothDeviceInfo&);
    void deviceScanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);

    // QLowEnergyController realted
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void deviceConnected();
    void errorReceived(QLowEnergyController::Error);
    void serviceScanDone();
    void deviceDisconnected();


    // QLowEnergyService related
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

Q_SIGNALS:
 //   void servicesUpdated();
    void characteristicsUpdated();
    void updateChanged();
    void stateChanged();
    void disconnected();
    void randomAddressChanged();
    void updateDisplay();


private:
    void setUpdate(const QString &message);
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    DeviceInfo currentDevice;
    QList<QObject *> devices;
    QList<QObject *> m_services;
    QList<QObject *> m_characteristics;
    QString m_previousAddress;
//    QString m_message;
    bool connected = false;
    QLowEnergyController *controller = nullptr;
    bool m_deviceScanState = false;
    bool randomAddress = false;

    void updateValue(const QLowEnergyCharacteristic &c, const QByteArray &value);

};

#endif // DEVICE_H
