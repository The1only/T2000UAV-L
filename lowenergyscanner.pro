TARGET = Transponder
INCLUDEPATH += .

QT += quick widgets charts quickwidgets sensors positioning multimedia multimediawidgets

#chatserver.cpp \

android: QT += serialport
android: QT += core-private
android: QT += core

macx: QT += serialport

ios: CONFIG+=sdk_no_version_check

macx {
    SOURCES += main.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp

    HEADERS += \
    QuickWidget.h \
    mainwindow.h \
    mytcpsocket.h \
    IOS_swift/WitSDK/Sensor/Modular/Processor/Roles/BWT901BLE5_0DataProcessor.swift \

}

ios{
    SOURCES += main.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp \


    HEADERS += \
    QuickWidget.h \
    mainwindow.h \
    mytcpsocket.h \
    IOS_swift/WitSDK/Sensor/Modular/Processor/Roles/BWT901BLE5_0DataProcessor.swift \

}

android {
    SOURCES += main.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp \
    lockhelper.cpp

    HEADERS += \
    QuickWidget.h \
    mainwindow.h \
    mytcpsocket.h \
    lockhelper.h

    ANDROID_PERMISSIONS += android.permission.WAKE_LOCK
    ANDROID_PERMISSIONS += android.permission.READ_EXTERNAL_STORAGE
    ANDROID_PERMISSIONS += android.permission.WRITE_EXTERNAL_STORAGE
    ANDROID_PERMISSIONS += android.permission.ACCESS_COARSE_LOCATION
    ANDROID_PERMISSIONS += android.permission.ACCESS_FINE_LOCATION
    ANDROID_PERMISSIONS += android.permission.ACCESS_WIFI_STATE
    ANDROID_PERMISSIONS += android.permission.USE_PERIPHERAL_IO
    ANDROID_PERMISSIONS += android.permission.CAMERA
    ANDROID_PERMISSIONS += android.permission.INTENT_ACTION_USB_PERMISSION

    ANDROID_PERMISSIONS += android.permission.INTERNET
    ANDROID_PERMISSIONS += android.permission.ACCESS_NETWORK_STATE
    ANDROID_PERMISSIONS += android.permission.FOREGROUND_SERVICE
    ANDROID_PERMISSIONS += android.hardware.usb.action.USB_DEVICE_ATTACHED
    ANDROID_PERMISSIONS += android.permission.WAKE_LOCK

    ANDROID_PERMISSIONS += android.permission.BLUETOOTH
    ANDROID_PERMISSIONS += android.permission.BLUETOOTH_ADMIN
    ANDROID_PERMISSIONS += android.permission.BLUETOOTH_SCAN
    ANDROID_PERMISSIONS += android.permission.BLUETOOTH_ADVERTISE
    ANDROID_PERMISSIONS += android.permission.BLUETOOTH_CONNECT

    QT += network
    QT += sensors
    QT += bluetooth
    QT += positioning

}

CONFIG += mobility

ios: QMAKE_INFO_PLIST = Info.plist
macos: QMAKE_INFO_PLIST = Info.qmake.macos.plist

#OTHER_FILES += assets/*.qml

#chatserver.h \

RESOURCES += \
    resources.qrc

INSTALLS += target

FORMS += \
    mainwindow_port_new.ui \
    remoteselector.ui \
    mainwindow_phone.ui \
    mainwindow_port_iPhone.ui \
    mainwindow_small.ui \
    mainwindow_port_small.ui \

#    mainwindow.ui \

ios | macx{
DISTFILES += \
    swift/BleExample/Assets.xcassets/AccentColor.colorset/Contents.json \
    swift/BleExample/Assets.xcassets/AppIcon.appiconset/Contents.json \
    swift/BleExample/Assets.xcassets/Contents.json \
    swift/BleExample/BleExampleApp.swift \
    swift/BleExample/Preview Content/Preview Assets.xcassets/Contents.json \
    swift/README.md \
    swift/WitSDK.xcodeproj/project.pbxproj \
    swift/WitSDK.xcodeproj/project.xcworkspace/contents.xcworkspacedata \
    swift/WitSDK.xcodeproj/project.xcworkspace/xcshareddata/IDEWorkspaceChecks.plist \
    swift/WitSDK.xcodeproj/project.xcworkspace/xcuserdata/terjenilsen.xcuserdatad/UserInterfaceState.xcuserstate \
    swift/WitSDK.xcodeproj/project.xcworkspace/xcuserdata/zhaowen.xcuserdatad/IDEFindNavigatorScopes.plist \
    swift/WitSDK.xcodeproj/project.xcworkspace/xcuserdata/zhaowen.xcuserdatad/UserInterfaceState.xcuserstate \
    swift/WitSDK.xcodeproj/xcuserdata/terjenilsen.xcuserdatad/xcschemes/xcschememanagement.plist \
    swift/WitSDK.xcodeproj/xcuserdata/zhaowen.xcuserdatad/xcdebugger/Breakpoints_v2.xcbkptlist \
    swift/WitSDK.xcodeproj/xcuserdata/zhaowen.xcuserdatad/xcschemes/xcschememanagement.plist \
    swift/WitSDK/Sensor/DeviceModel.swift \
    swift/WitSDK/Sensor/Errors/DeviceModelError.swift \
    swift/WitSDK/Sensor/Modular/Connector/Modular/BluetoothBLE.swift \
    swift/WitSDK/Sensor/Modular/Connector/Modular/IBluetoothEventObserver.swift \
    swift/WitSDK/Sensor/Modular/Connector/Modular/WitBluetoothManager.swift \
    swift/WitSDK/Sensor/Modular/Connector/Roles/WitCoreConnector.swift \
    swift/WitSDK/Sensor/Modular/Processor/Constants/WitSensorKey.swift \
    swift/WitSDK/Sensor/Modular/Processor/Protocol/IDataProcessor.swift \
    swift/WitSDK/Sensor/Modular/Processor/Roles/BWT901BLE5_0DataProcessor.swift \
    swift/WitSDK/Sensor/Modular/Resolver/Protocol/IProtocolResolver.swift \
    swift/WitSDK/Sensor/Modular/Resolver/Roles/BWT901BLE5_0ProtocolResolver.swift \
    swift/WitSDK/Sensor/Protocol/IKeyUpdateObserver.swift \
    swift/WitSDK/Sensor/Protocol/IListenKeyUpdateObserver.swift \
    swift/WitSDK/Sensor/Utils/CompareObjectHelper.swift \
    swift/WitSDK/Sensor/Utils/DataHelper.swift \
    swift/WitSDK/Sensor/Utils/DateHelper.swift \
    swift/WitSDK/Sensor/Utils/DipSensorMagHelper.swift \
    swift/WitSDK/Sensor/Utils/StringUtils.swift \
    swift/WitSDK/Sensor/Utils/SyncDictionary.swift \
    swift/WitSDK/WitSDK.docc/WitSDK.md \
    swift/WitSDK/WitSensorApi/Modular/Ble5/Bwt901ble.swift \
    swift/WitSDK/WitSensorApi/Modular/Ble5/IBwt901bleRecordObserver.swift
}

android {
DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/main/java/com/hoho/android/usbserial/driver/DeviceActivity.java \
    android/res/drawable/ic_delete_white_24dp.xml \
    android/res/drawable/ic_send_white_24dp.xml \
    android/res/menu/menu_devices.xml \
    android/res/menu/menu_terminal.xml \
    android/res/mipmap-hdpi/ic_launcher.png \
    android/res/mipmap-mdpi/ic_launcher.png \
    android/res/mipmap-xhdpi/ic_launcher.png \
    android/res/mipmap-xxhdpi/ic_launcher.png \
    android/res/mipmap-xxxhdpi/ic_launcher.png \
    android/res/values/arrays.xml \
    android/res/values/colors.xml \
    android/res/values/strings.xml \
    android/res/values/styles.xml \
    android/res/xml/device_filter.xml \
    android/src/main/java/com/hoho/android/usbserial/driver/CdcAcmSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/Ch34xSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/ChromeCcdSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/CommonUsbSerialPort.java \
    android/src/main/java/com/hoho/android/usbserial/driver/Cp21xxSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/CustomProber.java \
    android/src/main/java/com/hoho/android/usbserial/driver/FtdiSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/GsmModemSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/ProbeTable.java \
    android/src/main/java/com/hoho/android/usbserial/driver/ProlificSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/SerialTimeoutException.java \
    android/src/main/java/com/hoho/android/usbserial/driver/TestClassTerje.java \
    android/src/main/java/com/hoho/android/usbserial/driver/UsbId.java \
    android/src/main/java/com/hoho/android/usbserial/driver/UsbSerialDriver.java \
    android/src/main/java/com/hoho/android/usbserial/driver/UsbSerialPort.java \
    android/src/main/java/com/hoho/android/usbserial/driver/UsbSerialProber.java \
    android/src/main/java/com/hoho/android/usbserial/util/HexDump.java \
    android/src/main/java/com/hoho/android/usbserial/util/MonotonicClock.java \
    android/src/main/java/com/hoho/android/usbserial/util/SerialInputOutputManager.java \
    android/src/main/java/com/hoho/android/usbserial/util/UsbUtils.java \
    android/src/main/java/com/hoho/android/usbserial/util/XonXoffFilter.java \
    android/src/main/java/com/wit/witsdk/Bluetooth/WitBluetoothManager.java \
    android/src/main/java/com/wit/witsdk/Device/IMUActivity.java \
    android/src/main/java/com/wit/witsdk/Device/DeviceActivity.java \
    android/src/main/java/com/wit/witsdk/Device/DeviceEvent.java \
    android/src/main/java/com/wit/witsdk/Device/DeviceManager.java \
    android/src/main/java/com/wit/witsdk/Device/DeviceModel.java \
    android/src/main/java/com/wit/witsdk/Device/Interface/DeviceDataListener.java \
    android/src/main/java/com/wit/witsdk/Device/Interface/DeviceFindListener.java \
    android/src/main/java/com/wit/witsdk/UI/CustomAdapter.java \
    android/src/main/java/com/wit/witsdk/UI/ListItem.java \
}


contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

SUBDIRS += \
    android/WitSDK/consumer-rules.pro \
    android/WitSDK/proguard-rules.pro \
    android/imu/proguard-rules.pro



