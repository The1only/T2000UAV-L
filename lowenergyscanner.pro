
TEMPLATE = app
TARGET = lowenergyscanner
INCLUDEPATH += .
INCLUDEPATH += /Users/terjenilsen/Dropbox/Sportsfly/transponder/eigen-3.4.0
INCLUDEPATH += ./EKF_IMU_GPS/ekf_nav_ins/inc/
INCLUDEPATH += ./EKF_IMU_GPS/ekf_nav_ins/src/

DEFINES += QT_NO_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

QT += quick widgets charts quickwidgets sensors positioning multimediawidgets svgwidgets xml gui

macx {

    CONFIG += app_bundle
    QMAKE_INFO_PLIST = $$PWD/Info.plist

    QT += serialport multimedia bluetooth
    #opengl
    #openglwidgets

#    CONFIG += c++11
    CONFIG += c++17
    QMAKE_CXXFLAGS += -std=c++17

    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib -lpaho-mqttpp3 -lpaho-mqtt3as
    LIBS -= framework AGL

    SOURCES += main.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp \
    ./EKF_IMU_GPS/ekf_nav_ins/src/ekfNavINS.cpp \
    rotation_matrix.cpp \
    gpx_parse.cpp \
    mqttclient.cpp \
    serialport.cpp \
    INS_driver.cpp \
    BleUart.cpp \
    wit_c_sdk.c \


    HEADERS += \
    QuickWidget.h \
    mainwindow.h \
    mytcpsocket.h \
    ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS.h \
    ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS_quart.h \
    rotation_matrix.h \
    gpx_parse.h \
    mqttclient.h \
    serialport.h \
    BleUart.h \
    REG.h \
    wit_c_sdk.h \

#myNativeWrapperFunctions.h \
#MicroQiskitCpp.h \
#MicroQiskitCpp.cpp \

}

ios{
    CONFIG+=sdk_no_version_check
    QT += bluetooth

    #CONFIG += mobility

    # iOS frameworks typically added automatically, but ensure they’re there:
#    LIBS += \
#        -framework CoreMedia \
#        -framework CoreVideo \
#        -framework VideoToolbox \

#       -framework AVFoundation \
#       -framework AudioToolbox \

# Make sure we use the AVFoundation backend (QAVF*)
    # (For static builds this ensures the plugin is linked in.)
 #   QTPLUGIN += qavfmediaplayer qavfcamera

#    QTPLUGIN -= qffmpegmediaplugin
#    QTPLUGIN -= qffmpeg

    # iOS: Bundle identifier
    QMAKE_TARGET_BUNDLE_IDENTIFIER = teni@9tek.no

    # iOS: Manual signing setup
    QMAKE_MAC_SDK = iphoneos
    QMAKE_XCODE_CODE_SIGN_STYLE = Manual
    QMAKE_XCODE_DEVELOPMENT_TEAM = 75F9V574G2
    QMAKE_XCODE_CODE_SIGN_IDENTITY = "Apple Development: Terje Nilsen (MGWC6SDUJH)"
    QMAKE_XCODE_PROVISIONING_PROFILE = "terjenilsen-3.mobileprovision"

    SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp \
    ./EKF_IMU_GPS/ekf_nav_ins/src/ekfNavINS.cpp \
    myNativeWrapperFunctions.mm \
    rotation_matrix.cpp \
    gpx_parse.cpp \
    mqttclient.cpp \
    wit_c_sdk.c \
    INS_driver.cpp \

#MicroQiskitCpp.cpp \


    HEADERS += \
    mainwindow.h \
    mytcpsocket.h \
    ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS_quart.h \
    ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS.h \
    myNativeWrapperFunctions.h \
    rotation_matrix.h \
    $$PWD/example/Headers/WidgetAI.h \
    gpx_parse.h \
    mqttclient.h \
    serialport.h \
    REG.h \
    wit_c_sdk.h \

#   BleUart.h \

    PAHO_IOS_BASE = /Users/terjenilsen/Dropbox/Sportsfly/transponder/third_party/paho/ios
    #PAHO_IOS = $$PAHO_IOS_BASE/iphonesimulator
    PAHO_IOS = $$PAHO_IOS_BASE/iphoneos
    INCLUDEPATH += $$PAHO_IOS/include
    LIBS += -L$$PAHO_IOS/lib -lpaho-mqttpp3 -lpaho-mqtt3a

    # Recommended for static third-party on iOS
    QMAKE_CXXFLAGS += -fPIC

    QMAKE_INFO_PLIST = $$PWD/Info.plist
    QMAKE_BUNDLE_IDENTIFIER = teni@9tek.no

# QuickWidget.h \
# MicroQiskitCpp.h \
}

# Android-specific permissions
android {

    QT += serialport multimedia core-private core

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    RESOURCES += android/AndroidManifest.xml

    # Optional: override Android manifest attributes
    # QMAKE_ANDROID_PACKAGE_NAME = org.sportsfly.lowenergyscanner
    # QMAKE_ANDROID_VERSION_NAME = 1.0
    # QMAKE_ANDROID_VERSION_CODE = 1

    SOURCES += main.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp \
    lockhelper.cpp \
    ./EKF_IMU_GPS/ekf_nav_ins/src/ekfNavINS.cpp \
    rotation_matrix.cpp \
    gpx_parse.cpp \
    mqttclient.cpp \
    serialport_android.cpp \
    INS_driver.cpp \
    BleUart.cpp \
    wit_c_sdk.c \


#serialport_bluetooth.cpp \
#MicroQiskitCpp.cpp \

    HEADERS += \
    QuickWidget.h \
    mainwindow.h \
    mytcpsocket.h \
    lockhelper.h \
    ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS.h \
    ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS_quart.h \
    rotation_matrix.h \
    gpx_parse.h \
    mqttclient.h \
    serialport.h \
    BleUart.h \
    REG.h \

#serialport_bluetooth.h \
#MicroQiskitCpp.h \

    PAHO_ROOT = /Users/terjenilsen/Dropbox/Sportsfly/transponder/third_party/paho/$$ANDROID_TARGET_ARCH
    INCLUDEPATH += $$PAHO_ROOT/include
    LIBS += -L$$PAHO_ROOT/lib -lpaho-mqttpp3 -lpaho-mqtt3a

    # Helpful for static libs:
    QMAKE_CXXFLAGS += -fPIC

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

    QMAKE_CXXFLAGS += -std=c++17
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    CONFIG += c++17
    CONFIG += mobility
    CONFIG +=debug
    MOBILITY += bluetooth
}

ios: QMAKE_INFO_PLIST = Info.plist
macos: QMAKE_INFO_PLIST = Info.qmake.macos.plist

RESOURCES += \
    resources.qrc

INSTALLS += target

FORMS += \
    mainwindow_port_new.ui \
    mainwindow_port_small.ui \
    mainwindow_port_vertical.ui \
    remoteselector.ui \
    mainwindow_phone.ui \
    mainwindow_port_iPhone.ui \
    mainwindow_small.ui \

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

}

#android/src/main/java/com/hoho/android/usbserial/driver/NmeaActivity.java \

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

SUBDIRS += \
    android/WitSDK/consumer-rules.pro \
    android/WitSDK/proguard-rules.pro \
    android/imu/proguard-rules.pro


include($$PWD/qfi/qfi.pri)
include($$PWD/example/example.pri)

#MicroQiskitCpp.h \
#MicroQiskitCpp.cpp \


DISTFILES += \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml



