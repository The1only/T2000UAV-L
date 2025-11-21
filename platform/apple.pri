# platform/apple.pri
# macOS and iOS settings

# ----------------
# macOS
# ----------------
macx {
    message(This is MAC....)

    CONFIG += app_bundle
    QMAKE_INFO_PLIST = $$PWD/Info.plist

    QT += serialport multimedia bluetooth

    # C++ standard
    CONFIG += c++17
    QMAKE_CXXFLAGS += -std=c++17

    message("Using paho.mqtt.cpp and paho.mqtt.c")
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib

    # You had both of these; keep just one set
    LIBS += -lpaho-mqttpp3 -lpaho-mqtt3a
    LIBS -= framework AGL

    # prefer opt/homebrew on Apple Silicon
    exists($$PWD/../GeographicLib) {
        message(... FOUND ... )
        INCLUDEPATH += $$PWD/../GeographicLib/build-mac/install/include
        LIBS += -L$$PWD/../GeographicLib/build-mac/install/lib -lGeographicLib
        GEODATA_DIR = //usr/local/share/GeographicLib
#       INCLUDEPATH += /opt/homebrew/include
#       LIBS += -L/opt/homebrew/lib -lGeographicLib
#       GEODATA_DIR = /opt/homebrew/share/GeographicLib
    } else {
        message(... ERROR ... Missing: $$PWD/../GeographicLib/build-mac/install/include)
    }

    DEFINES += USE_GEOGRAPHICLIB

    SOURCES += \
        ./Source/driver/bleuart.cpp \
        ./Source/generic/main.cpp \
        ./Source/generic/mainwindow.cpp \
        ./Source/generic/mytcpsocket.cpp \
        ./EKF_IMU_GPS/ekf_nav_ins/src/ekfNavINS.cpp \
        ./Source/driver/rotation_matrix.cpp \
        ./Source/driver/gpx_parse.cpp \
        ./Source/driver/mqttclient.cpp \
        ./Source/driver/serialport.cpp \
        ./Source/generic/INS_driver.cpp \
        ./Source/driver/wit_c_sdk.c \
        ./Source/driver/geoid_helper.cpp


    HEADERS += \
        ./Header/driver/bleuart.h \
        ./Header/generic/QuickWidget.h \
        ./Header/generic/mainwindow.h \
        ./Header/generic/mytcpsocket.h \
        ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS.h \
        ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS_quart.h \
        ./Header/driver/rotation_matrix.h \
        ./Header/driver/gpx_parse.h \
        ./Header/driver/mqttclient.h \
        ./Header/driver/serialport.h \
        ./Header/generic/REG.h \
        ./Header/driver/wit_c_sdk.h \
        ./Header/driver/geoid_helper.h
}

# ----------------
# iOS
# ----------------
ios {
    message(This is iPhone....)

    CONFIG += sdk_no_version_check
    QT += bluetooth

    # iOS bundle / signing
    QMAKE_TARGET_BUNDLE_IDENTIFIER = teni@9tek.no
    QMAKE_MAC_SDK = iphoneos
    QMAKE_XCODE_CODE_SIGN_STYLE = Manual
    QMAKE_XCODE_DEVELOPMENT_TEAM = 75F9V574G2
    QMAKE_XCODE_CODE_SIGN_IDENTITY = "Apple Development: Terje Nilsen (MGWC6SDUJH)"
    QMAKE_XCODE_PROVISIONING_PROFILE = "terjenilsen-3.mobileprovision"

    # iOS sources
    SOURCES += \
        ./Source/generic/main.cpp \
        ./Source/generic/mainwindow.cpp \
        ./Source/generic/mytcpsocket.cpp \
        ./EKF_IMU_GPS/ekf_nav_ins/src/ekfNavINS.cpp \
        ./Source/generic/myNativeWrapperFunctions.mm \
        ./Source/driver/rotation_matrix.cpp \
        ./Source/driver/gpx_parse.cpp \
        ./Source/generic/mqttclient.cpp \
        ./Source/driver/wit_c_sdk.c \
        ./Source/generic/INS_driver.cpp

    HEADERS += \
        ./Header/generic/mainwindow.h \
        ./Header/generic/mytcpsocket.h \
        ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS_quart.h \
        ./EKF_IMU_GPS/ekf_nav_ins/inc/ekfNavINS.h \
        ./Header/generic/myNativeWrapperFunctions.h \
        ./Header/driver/rotation_matrix.h \
        $$PWD/example/Headers/WidgetAI.h \
        ./Header/driver/gpx_parse.h \
        ./Header/driver/mqttclient.h \
        ./Header/driver/serialport.h \
        ./Header/generic/REG.h \
        ./Header/driver/wit_c_sdk.h

    # your iOS paho path
    PAHO_IOS_BASE = /Users/terjenilsen/Dropbox/Sportsfly/transponder/third_party/paho/ios
    PAHO_IOS = $$PAHO_IOS_BASE/iphoneos
    INCLUDEPATH += $$PAHO_IOS/include
    LIBS += -L$$PAHO_IOS/lib -lpaho-mqttpp3 -lpaho-mqtt3a

    # Position plist for iOS
    QMAKE_INFO_PLIST = $$PWD/Info.plist
    QMAKE_BUNDLE_IDENTIFIER = teni@9tek.no

    # Recommended for static 3rd party on iOS
    QMAKE_CXXFLAGS += -fPIC
}

# Optional: if you still want these convenience lines:
ios: QMAKE_INFO_PLIST = Info.plist
macos: QMAKE_INFO_PLIST = Info.qmake.macos.plist

