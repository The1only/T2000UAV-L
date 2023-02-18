TARGET = wlanscanner
INCLUDEPATH += .

QT += quick widgets

# Input
SOURCES += main.cpp \
    chatclient.cpp \
    chatserver.cpp \
    mainwindow.cpp \
    mytcpsocket.cpp \
    remoteselector.cpp

QT += network
QT += serialport
QT += sensors
QT += bluetooth

ios: QMAKE_INFO_PLIST = Info.plist
macos: QMAKE_INFO_PLIST = Info.qmake.macos.plist

#OTHER_FILES += assets/*.qml

HEADERS += \
    chatclient.h \
    chatserver.h \
    mainwindow.h \
    mytcpsocket.h \
    remoteselector.h

RESOURCES += \
    resources.qrc

INSTALLS += target

FORMS += \
    mainwindow.ui \
    mainwindow_small.ui \
    mainwindow_port.ui \
    remoteselector.ui

# mainwindow_phone.ui \

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/usb_device_filter.xml \
    android/res/xml/usb_device_filter.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
