/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "device.h"
#include "serviceinfo.h"
#include "characteristicinfo.h"
#include <iostream>

#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <QDebug>
#include <QList>
#include <QMetaEnum>
#include <QThread>
#include <QTimer>

#include <QtWidgets/qapplication.h>
#include <QtCore/QLoggingCategory>
#include <stdio.h>

#define key "TransponderKey"
static QString const channels[] = { "{6e400002-b5a3-f393-e0a9-e50e24dcca9e}", "{6e400003-b5a3-f393-e0a9-e50e24dcca9e}"};

Device::Device()
{
    //! [les-devicediscovery-1]
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    discoveryAgent->setLowEnergyDiscoveryTimeout(1500);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &Device::addDevice);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &Device::deviceScanError);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &Device::deviceScanFinished);
    //! [les-devicediscovery-1]

    setUpdate("Search");
}

Device::~Device()
{
    delete l_service;
    delete discoveryAgent;
    delete controller;
    qDeleteAll(devices);
    qDeleteAll(m_services);
    // qDeleteAll(m_characteristics);
    devices.clear();
    m_services.clear();
    //m_characteristics.clear();
}

void Device::setVal()
{
    QByteArray array = m_message.toLocal8Bit();
    char* buffer = array.data();
    std::fprintf(stderr,"0: %s \n\r",buffer);  //m_message);
}

void Device::startDeviceDiscovery()
{
    qDeleteAll(devices);
    devices.clear();

    setUpdate("Scanning for devices ...");
    //! [les-devicediscovery-2]
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    //! [les-devicediscovery-2]

    if (discoveryAgent->isActive()) {
        m_deviceScanState = true;
        Q_EMIT stateChanged();
    }
}

//! [les-devicediscovery-3]
void Device::addDevice(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration){
        QString s = info.name();
        key2 = QString(info.deviceUuid().toString());

        if (!s.compare(key)){
            setUpdate("Device added: " + info.name() + "\nID: " + info.deviceUuid().toString());
        }
    }
}
//! [les-devicediscovery-3]

void Device::deviceScanFinished()
{
    setUpdate("Scanning...");
    const QList<QBluetoothDeviceInfo> foundDevices = discoveryAgent->discoveredDevices();
    for (auto &nextDevice : foundDevices){
        if (nextDevice.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration){
            DeviceInfo *n = new DeviceInfo(nextDevice);
            if (!n->getName().compare(key)){
                devices.append(n);
                break;
            }
            delete(n);
        }
    }

    m_deviceScanState = false;
    emit stateChanged();
    if (devices.isEmpty())
        setUpdate("No Low Energy devices found...");
    else
        setUpdate("Done!");

    scanServices(key);

}

void Device::scanServices(const QString &address)
{
    setUpdate("scanServices...");

    for (auto d: qAsConst(devices)) {
        if (auto device = qobject_cast<DeviceInfo *>(d)) {
            if (device->getName() == address ) {
                QByteArray array = device->getAddress().toLocal8Bit();
                char* buffer = array.data();
                std::fprintf(stderr,"Found: %s \n\r",buffer);
                currentDevice.setDevice(device->getDevice());
                break;
            }
        }
    }

    if (!currentDevice.getDevice().isValid()) {
        qWarning() << "Not a valid device";
        return;
    }
    setUpdate("Device valid...");

    emit characteristicsUpdated();
    qDeleteAll(m_services);
    m_services.clear();

    setUpdate("Disconnecting from device...");

    if (controller && m_previousAddress != currentDevice.getAddress()) {
        controller->disconnectFromDevice();
        delete controller;
        controller = nullptr;
        setUpdate("scanServices\n(Wrong device...)");
    }

    setUpdate("Connecting to processes...");
    //! [les-controller-1]
    if (!controller) {
        setUpdate("Connecting to controller...");

        // Connecting signals and slots for connecting to LE services.
        controller = QLowEnergyController::createCentral(currentDevice.getDevice(), this);

        connect(controller, &QLowEnergyController::connected, this, &Device::deviceConnected);
        connect(controller, &QLowEnergyController::errorOccurred, this, &Device::errorReceived);
        connect(controller, &QLowEnergyController::disconnected, this, &Device::deviceDisconnected);
        connect(controller, &QLowEnergyController::serviceDiscovered, this, &Device::addLowEnergyService);
        connect(controller, &QLowEnergyController::discoveryFinished, this, &Device::serviceScanDone);
    }

    setUpdate("Set address type...");
    controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

    setUpdate("Conecting to devices...");
    controller->connectToDevice();
    //! [les-controller-1]

    m_previousAddress = currentDevice.getAddress();
}

void Device::addLowEnergyService(const QBluetoothUuid &serviceUuid)
{
    //! [les-service-1]
    QLowEnergyService *service = controller->createServiceObject(serviceUuid);
    if (!service) {
        qWarning() << "Cannot create service for uuid";
        return;
    }
    //! [les-service-1]

    auto serv = new ServiceInfo(service);
    m_services.append(serv);

    connectToService(key2);
}
//! [les-service-1]F

void Device::serviceScanDone()
{
    setUpdate("Back\n(Service scan done!)");
    // force UI in case we didn't find anything
    if (m_services.isEmpty())
    {
        //      emit servicesUpdated();
    }
}

void Device::connectToService(const QString &uuid)
{
    setUpdate("Find services...");
    QLowEnergyService *service = nullptr;
    for (auto s: qAsConst(m_services)) {
        auto serviceInfo = qobject_cast<ServiceInfo *>(s);
        if (!serviceInfo)
            continue;

        QString x = serviceInfo->getUuid();
        if (x.compare(uuid)) {
            service = serviceInfo->service();
            break;
        }
    }

    if (!service)
        return;

    setUpdate("Connecting to services...)");
    emit characteristicsUpdated();

    if (service->state() == QLowEnergyService::RemoteService) {
        //! [les-service-3]

        service->discoverDetails();
        connect(service, &QLowEnergyService::stateChanged, this, &Device::serviceDetailsDiscovered);
        connect(service, &QLowEnergyService::characteristicChanged, this, &Device::updateValue);
        connect(service, &QLowEnergyService::descriptorWritten, this, &Device::updateValueSet);

        setUpdate("Discovering details...");



        //! [les-service-3]
    }

    QTimer::singleShot(100, this, &Device::characteristicsUpdated);
}

void Device::updateValueSet()
{
    setUpdate("Data sent...");

}
void Device::deviceConnected()
{
    setUpdate("Discovering services...");
    connected = true;
    //! [les-service-2]

    controller->discoverServices();
    //! [les-service-2]
}

void Device::errorReceived(QLowEnergyController::Error /*error*/)
{
    qWarning() << "Error: " << controller->errorString();
    setUpdate(QString("Back\n(%1)").arg(controller->errorString()));
}

void Device::setUpdate(const QString &message)
{
    m_message = message;
    emit updateChanged();
}

void Device::disconnectFromDevice()
{
    if (controller->state() != QLowEnergyController::UnconnectedState)
        controller->disconnectFromDevice();
    else
        deviceDisconnected();
}

void Device::deviceDisconnected()
{
    qWarning() << "Disconnect from device";
    setUpdate("Disconnect from device");
    emit disconnected();
}

void Device::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    if (newState != QLowEnergyService::RemoteServiceDiscovered) {
        if (newState != QLowEnergyService::RemoteServiceDiscovering) {
            QMetaObject::invokeMethod(this, "characteristicsUpdated", Qt::QueuedConnection);
        }
        return;
    }

    auto service = qobject_cast<QLowEnergyService *>(sender());
    l_service = service;

    if (!l_service)
        return;

    //! [les-chars]
    const QList<QLowEnergyCharacteristic> chars = l_service->characteristics();
    for (const QLowEnergyCharacteristic &ch : chars) {
        auto cInfo = new CharacteristicInfo(ch);
        setUpdate(cInfo->getPermission());
    }
    //! [les-chars]

    QTimer::singleShot(1000, this, &Device::test);

}

void Device::test()
{
    setUpdate("Command open...");
    SendData("Command open...",0);
}

void Device::updateValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    QString x = c.uuid().toString();

    if (!channels[0].compare(x)){
        setUpdate("ch1: "+value);
        return;
    }
    if (!channels[1].compare(x)){

        setUpdate("ch2: "+value);

        receivedData = value;
        emit updateDisplay();

        return;
    }
}

void Device::SendData(QString data, int chanel)
{
    if (!l_service)
        return;

    const QList<QLowEnergyCharacteristic> chars = l_service->characteristics();
    for (const QLowEnergyCharacteristic &ch : chars) {
        auto cInfo = new CharacteristicInfo(ch);
        QString y = "{"+cInfo->getUuid()+"}";

        if (!channels[chanel].compare(y)){
            QByteArray x = data.toUtf8();
            l_service->writeCharacteristic(ch, x);
        }
    }
}


void Device::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setUpdate("The Bluetooth adaptor is powered off, power it on before doing discovery.");
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setUpdate("Writing or reading from the device resulted in an error.");
    else {
        static QMetaEnum qme = discoveryAgent->metaObject()->enumerator(
                    discoveryAgent->metaObject()->indexOfEnumerator("Error"));
        setUpdate("Error: " + QLatin1String(qme.valueToKey(error)));
    }

    m_deviceScanState = false;
    emit stateChanged();
}

bool Device::state()
{
    return m_deviceScanState;
}
