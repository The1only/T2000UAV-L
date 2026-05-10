TEMPLATE = app
TARGET = lowenergyscanner

# ---- common Qt modules ----
QT += quick widgets charts quickwidgets sensors positioning multimediawidgets svgwidgets xml gui

# ---- common include paths ----
INCLUDEPATH += .
INCLUDEPATH += /Users/terjenilsen/Dropbox/Sportsfly/transponder/eigen-3.4.0
INCLUDEPATH += ./EKF_IMU_GPS/ekf_nav_ins/inc/
INCLUDEPATH += ./EKF_IMU_GPS/ekf_nav_ins/src/

# ---- common defines ----
DEFINES += QT_NO_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# ---- common resources ----
RESOURCES += \
    resources.qrc

# ---- common ui forms ----
FORMS += \
    mainwindow_port_new.ui \
    mainwindow_port_small.ui \
    mainwindow_port_vertical.ui \
    remoteselector.ui \
    mainwindow_phone.ui \
    mainwindow_port_iPhone.ui \
    mainwindow_small.ui

# ---- common installs (optional) ----
INSTALLS += target

# ---- common extra .pri you already include ----
include($$PWD/qfi/qfi.pri)
include($$PWD/example/example.pri)

# ---- platform-specific includes ----
macx|ios {
    include($$PWD/platform/apple.pri)
}

android {
    include($$PWD/platform/android.pri)
}

DISTFILES += \
    platform/android.pri \
    platform/apple.pri

