/********************************************************************************
** Form generated from reading UI file 'mainwindow_port_iPhone.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_PORT_IPHONE_H
#define UI_MAINWINDOW_PORT_IPHONE_H

#include <QtCore/QVariant>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDial>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QWidget>
#include <example/WidgetEHSI.h>
#include "example/WidgetAI.h"
#include "example/WidgetALT.h"
#include "example/WidgetASI.h"
#include "example/WidgetEADI.h"
#include "example/WidgetHI.h"
#include "example/WidgetTC.h"
#include "example/WidgetVSI.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow_port_iPhone
{
public:
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    QWidget *page_2;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;
    QPushButton *pushButton_16;
    QPushButton *pushButton_17;
    QPushButton *pushButton_18;
    QPushButton *pushButton_14;
    QPushButton *pushButton_11;
    QTabWidget *tabWidget;
    QWidget *tab;
    QLabel *label_2;
    QLCDNumber *lcdNumber_3;
    QWidget *tab_2;
    QPlainTextEdit *plainTextEdit;
    QWidget *tab_3;
    QPushButton *pushButton_19;
    QPushButton *pushButton_off;
    QPushButton *pushButton_12;
    QPushButton *pushButton_13;
    QPushButton *reconnect_now;
    QLCDNumber *lcdNumber;
    QPushButton *pushButton_Ident;
    QPushButton *pushButton_stby;
    QPushButton *pushButton_norm;
    QPushButton *pushButton_alt;
    QLCDNumber *lcdNumber_2;
    QPushButton *pushButton_10;
    QPushButton *select_gyro_page;
    QPushButton *select_camera_from_transponder;
    QLabel *label_17;
    QLabel *label_3;
    QWidget *page;
    QLabel *label_4;
    QTimeEdit *timeEdit;
    QLabel *label_5;
    QTimeEdit *timeEdit_2;
    QLabel *label_6;
    QGraphicsView *graphicsView;
    QLabel *compass;
    QLabel *speed;
    QLabel *roll;
    QLabel *pitch;
    QPushButton *select_dumy_page2;
    QPushButton *select_transponder_page;
    QPushButton *timer_start;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *altitude;
    QLabel *temp;
    QLabel *label_12;
    QPlainTextEdit *listView;
    QLabel *label_15;
    QLabel *temperature;
    QLabel *baro_alt;
    QLabel *label_16;
    QWidget *page_6;
    QPushButton *select_transponder_page_2;
    QPushButton *select_dumy_page2_2;
    WidgetAI *widgetai;
    WidgetTC *widgetTC;
    WidgetALT *widgetALT;
    WidgetASI *widgetASI;
    WidgetVSI *widgetVSI;
    WidgetHI *widgetHI;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QRadioButton *radioButton_3;
    QLabel *label_18;
    QLabel *label_19;
    QLabel *label_20;
    QDial *dial;
    QLineEdit *doubleSpinBox;
    QWidget *page_7;
    QPushButton *select_transponder_page_3;
    QPushButton *select_from_4_to_5;
    WidgetEADI *widgetEADI;
    WidgetEHSI *widgetEHSI;
    QDial *dial_2;
    QLineEdit *doubleSpinBox_2;
    QWidget *page_9;
    QPushButton *select_transponder_page_4;
    QPushButton *select_from_4_to_6;
    QLabel *label_22;
    QLabel *radar_speed;
    QLabel *label_27;
    QLabel *label_26;
    QGraphicsView *graphicsView_2;
    QLabel *label_28;
    QLabel *label_29;
    QWidget *page_3;
    QPushButton *select_transponder_page2;
    QPushButton *select_gyro_page2;
    QTextEdit *textEdit;
    QTextEdit *textEdit_2;
    QLabel *label_13;
    QLabel *label_14;
    QPushButton *pushButton_15;
    QWidget *page_4;
    QPushButton *select_transponder_page2_2;
    QPushButton *select_gyro_page2_2;
    QPushButton *fly_home;
    QWidget *page_8;
    QPushButton *select_transponder_page_camera;
    QPushButton *select_page2_map;
    QPushButton *reset_att;
    QPushButton *reset_heading;
    QPushButton *imu_reset;
    QPushButton *use_hw;
    QPushButton *exit_2;
    QPushButton *use_gps_in_attitude;
    QLabel *label;
    QLabel *label_21;
    QLabel *label_23;
    QPushButton *reset_altitude_2;
    QPushButton *reset_heading_3;
    QLabel *label_24;
    QPushButton *use_built_inn_barometer;
    QWidget *page_5;
    QPushButton *select_transponder_page2_3;
    QPushButton *select_gyro_page2_3;
    QVideoWidget *viewfinder;
    QPushButton *pushButton_20;
    QPushButton *pushButton_21;
    QPushButton *pushButton_22;
    QPushButton *pushButton_23;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow_port_iPhone)
    {
        if (MainWindow_port_iPhone->objectName().isEmpty())
            MainWindow_port_iPhone->setObjectName("MainWindow_port_iPhone");
        MainWindow_port_iPhone->resize(745, 380);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow_port_iPhone->sizePolicy().hasHeightForWidth());
        MainWindow_port_iPhone->setSizePolicy(sizePolicy);
        MainWindow_port_iPhone->setMinimumSize(QSize(745, 380));
        MainWindow_port_iPhone->setMaximumSize(QSize(745, 380));
        MainWindow_port_iPhone->setStyleSheet(QString::fromUtf8("background-color: rgb(0,0,0);"));
        MainWindow_port_iPhone->setUnifiedTitleAndToolBarOnMac(false);
        centralwidget = new QWidget(MainWindow_port_iPhone);
        centralwidget->setObjectName("centralwidget");
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(0, 0, 741, 371));
        stackedWidget->setStyleSheet(QString::fromUtf8(""));
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        pushButton = new QPushButton(page_2);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(10, 10, 71, 71));
        pushButton->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/one.png); \n"
"    background-image: url(:/one.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/one.png);\n"
"    background-image: url(:/one.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_2 = new QPushButton(page_2);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(90, 10, 71, 71));
        pushButton_2->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/two.png); \n"
"    background-image: url(:/two.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/two.png);\n"
"    background-image: url(:/two.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_3 = new QPushButton(page_2);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(170, 10, 71, 71));
        pushButton_3->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/tree.png); \n"
"    background-image: url(:/tree.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/tree.png);\n"
"    background-image: url(:/tree.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_4 = new QPushButton(page_2);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(10, 90, 71, 71));
        pushButton_4->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/fore.png); \n"
"    background-image: url(:/fore.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/fore.png);\n"
"    background-image: url(:/fore.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_5 = new QPushButton(page_2);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(90, 90, 71, 71));
        pushButton_5->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/five.png); \n"
"    background-image: url(:/five.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/five.png);\n"
"    background-image: url(:/five.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_6 = new QPushButton(page_2);
        pushButton_6->setObjectName("pushButton_6");
        pushButton_6->setGeometry(QRect(170, 90, 71, 71));
        pushButton_6->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/six.png); \n"
"    background-image: url(:/six.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/six.png);\n"
"    background-image: url(:/six.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_7 = new QPushButton(page_2);
        pushButton_7->setObjectName("pushButton_7");
        pushButton_7->setGeometry(QRect(10, 170, 71, 71));
        pushButton_7->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/seven.png); \n"
"    background-image: url(:/seven.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/seven.png);\n"
"    background-image: url(:/seven.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_8 = new QPushButton(page_2);
        pushButton_8->setObjectName("pushButton_8");
        pushButton_8->setGeometry(QRect(90, 170, 71, 71));
        pushButton_8->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/eight.png); \n"
"    background-image: url(:/eight.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/eight.png);\n"
"    background-image: url(:/eight.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_9 = new QPushButton(page_2);
        pushButton_9->setObjectName("pushButton_9");
        pushButton_9->setGeometry(QRect(170, 170, 71, 71));
        pushButton_9->setStyleSheet(QString::fromUtf8("\n"
"QPushButton {\n"
"    image: url(:/nine.png); \n"
"    background-image: url(:/nine.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/nine.png);\n"
"    background-image: url(:/nine.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_16 = new QPushButton(page_2);
        pushButton_16->setObjectName("pushButton_16");
        pushButton_16->setGeometry(QRect(10, 250, 71, 71));
        QFont font;
        font.setBold(true);
        pushButton_16->setFont(font);
        pushButton_16->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    image: url(:/enter.png); \n"
"    background-image: url(:/enter.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/enter.png);\n"
"    background-image: url(:/enter.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_17 = new QPushButton(page_2);
        pushButton_17->setObjectName("pushButton_17");
        pushButton_17->setGeometry(QRect(90, 250, 71, 71));
        pushButton_17->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    image: url(:/zero.png); \n"
"    background-image: url(:/zero.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/zero.png);\n"
"    background-image: url(:/zero.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_18 = new QPushButton(page_2);
        pushButton_18->setObjectName("pushButton_18");
        pushButton_18->setGeometry(QRect(170, 250, 71, 71));
        pushButton_18->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    image: url(:/del.png); \n"
"    background-image: url(:/del.png);  \n"
"    background-color: rgb(49, 49, 49);\n"
"}\n"
"QPushButton:pressed {\n"
"    image: url(:/del.png);\n"
"    background-image: url(:/del.png);\n"
"    background-color: rgb(224, 0, 0);\n"
"}"));
        pushButton_14 = new QPushButton(page_2);
        pushButton_14->setObjectName("pushButton_14");
        pushButton_14->setGeometry(QRect(10, 330, 71, 21));
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(true);
        pushButton_14->setFont(font1);
        pushButton_14->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(0, 0, 0);\n"
"border: 2px solid #505;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #900\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"\n"
""));
        pushButton_11 = new QPushButton(page_2);
        pushButton_11->setObjectName("pushButton_11");
        pushButton_11->setGeometry(QRect(90, 330, 71, 21));
        pushButton_11->setFont(font1);
        pushButton_11->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(0, 0, 0);\n"
"border: 2px solid #505;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #900\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"\n"
""));
        pushButton_11->setCheckable(false);
        tabWidget = new QTabWidget(page_2);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(250, 170, 291, 161));
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Times New Roman")});
        font2.setPointSize(22);
        tabWidget->setFont(font2);
        tabWidget->setStyleSheet(QString::fromUtf8("color: rgb(155, 155, 155);\n"
"selection-color: rgb(253, 255, 135);\n"
"\n"
""));
        tab = new QWidget();
        tab->setObjectName("tab");
        label_2 = new QLabel(tab);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(220, 100, 61, 31));
        QFont font3;
        font3.setPointSize(22);
        font3.setItalic(true);
        label_2->setFont(font3);
        label_2->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        lcdNumber_3 = new QLCDNumber(tab);
        lcdNumber_3->setObjectName("lcdNumber_3");
        lcdNumber_3->setGeometry(QRect(0, 0, 281, 111));
        lcdNumber_3->setStyleSheet(QString::fromUtf8("color: rgb(13, 255, 252);"));
        lcdNumber_3->setSmallDecimalPoint(false);
        lcdNumber_3->setDigitCount(5);
        lcdNumber_3->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber_3->setProperty("intValue", QVariant(88888));
        tabWidget->addTab(tab, QString());
        lcdNumber_3->raise();
        label_2->raise();
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        plainTextEdit = new QPlainTextEdit(tab_2);
        plainTextEdit->setObjectName("plainTextEdit");
        plainTextEdit->setGeometry(QRect(0, 0, 271, 121));
        QFont font4;
        font4.setPointSize(14);
        plainTextEdit->setFont(font4);
        plainTextEdit->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        plainTextEdit->setReadOnly(true);
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        pushButton_19 = new QPushButton(tab_3);
        pushButton_19->setObjectName("pushButton_19");
        pushButton_19->setGeometry(QRect(160, 85, 111, 41));
        QFont font5;
        font5.setPointSize(16);
        font5.setBold(true);
        pushButton_19->setFont(font5);
        pushButton_19->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        pushButton_off = new QPushButton(tab_3);
        pushButton_off->setObjectName("pushButton_off");
        pushButton_off->setGeometry(QRect(20, 85, 111, 41));
        pushButton_off->setFont(font5);
        pushButton_off->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        pushButton_12 = new QPushButton(tab_3);
        pushButton_12->setObjectName("pushButton_12");
        pushButton_12->setGeometry(QRect(160, 0, 111, 41));
        pushButton_12->setFont(font5);
        pushButton_12->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        pushButton_13 = new QPushButton(tab_3);
        pushButton_13->setObjectName("pushButton_13");
        pushButton_13->setGeometry(QRect(20, 0, 111, 41));
        pushButton_13->setFont(font5);
        pushButton_13->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        reconnect_now = new QPushButton(tab_3);
        reconnect_now->setObjectName("reconnect_now");
        reconnect_now->setGeometry(QRect(90, 43, 111, 40));
        reconnect_now->setFont(font5);
        reconnect_now->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        tabWidget->addTab(tab_3, QString());
        lcdNumber = new QLCDNumber(page_2);
        lcdNumber->setObjectName("lcdNumber");
        lcdNumber->setGeometry(QRect(240, 0, 321, 161));
        lcdNumber->setFont(font);
        lcdNumber->setStyleSheet(QString::fromUtf8("color: rgb(0, 223, 15);"));
        lcdNumber->setDigitCount(4);
        lcdNumber->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber->setProperty("intValue", QVariant(8888));
        pushButton_Ident = new QPushButton(page_2);
        pushButton_Ident->setObjectName("pushButton_Ident");
        pushButton_Ident->setGeometry(QRect(570, 10, 101, 51));
        QFont font6;
        font6.setPointSize(25);
        font6.setBold(true);
        pushButton_Ident->setFont(font6);
        pushButton_Ident->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        pushButton_stby = new QPushButton(page_2);
        pushButton_stby->setObjectName("pushButton_stby");
        pushButton_stby->setGeometry(QRect(570, 70, 101, 51));
        pushButton_stby->setFont(font6);
        pushButton_stby->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(255, 166, 20);\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #2A0\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        pushButton_norm = new QPushButton(page_2);
        pushButton_norm->setObjectName("pushButton_norm");
        pushButton_norm->setGeometry(QRect(570, 130, 101, 51));
        pushButton_norm->setFont(font6);
        pushButton_norm->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        pushButton_alt = new QPushButton(page_2);
        pushButton_alt->setObjectName("pushButton_alt");
        pushButton_alt->setGeometry(QRect(570, 190, 101, 51));
        pushButton_alt->setFont(font6);
        pushButton_alt->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        lcdNumber_2 = new QLCDNumber(page_2);
        lcdNumber_2->setObjectName("lcdNumber_2");
        lcdNumber_2->setGeometry(QRect(540, 260, 131, 91));
        QFont font7;
        font7.setItalic(true);
        lcdNumber_2->setFont(font7);
        lcdNumber_2->setStyleSheet(QString::fromUtf8("color: rgb(205, 243, 58);"));
        lcdNumber_2->setDigitCount(4);
        lcdNumber_2->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber_2->setProperty("intValue", QVariant(8888));
        pushButton_10 = new QPushButton(page_2);
        pushButton_10->setObjectName("pushButton_10");
        pushButton_10->setGeometry(QRect(170, 330, 71, 21));
        QFont font8;
        font8.setPointSize(11);
        font8.setBold(true);
        pushButton_10->setFont(font8);
        pushButton_10->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(0, 0, 0);\n"
"border: 2px solid #505;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #900\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"\n"
""));
        pushButton_10->setCheckable(false);
        select_gyro_page = new QPushButton(page_2);
        select_gyro_page->setObjectName("select_gyro_page");
        select_gyro_page->setGeometry(QRect(680, 10, 51, 161));
        QFont font9;
        font9.setPointSize(19);
        font9.setBold(true);
        select_gyro_page->setFont(font9);
        select_gyro_page->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_camera_from_transponder = new QPushButton(page_2);
        select_camera_from_transponder->setObjectName("select_camera_from_transponder");
        select_camera_from_transponder->setGeometry(QRect(680, 180, 51, 161));
        select_camera_from_transponder->setFont(font9);
        select_camera_from_transponder->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        label_17 = new QLabel(page_2);
        label_17->setObjectName("label_17");
        label_17->setGeometry(QRect(500, 150, 71, 20));
        QFont font10;
        font10.setPointSize(15);
        font10.setItalic(true);
        label_17->setFont(font10);
        label_17->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_17->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_3 = new QLabel(page_2);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(270, 330, 151, 21));
        QFont font11;
        font11.setPointSize(17);
        font11.setItalic(true);
        label_3->setFont(font11);
        label_3->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        stackedWidget->addWidget(page_2);
        pushButton->raise();
        pushButton_2->raise();
        pushButton_3->raise();
        pushButton_4->raise();
        pushButton_5->raise();
        pushButton_6->raise();
        pushButton_7->raise();
        pushButton_8->raise();
        pushButton_9->raise();
        pushButton_16->raise();
        pushButton_17->raise();
        pushButton_18->raise();
        pushButton_14->raise();
        pushButton_11->raise();
        lcdNumber->raise();
        pushButton_Ident->raise();
        pushButton_stby->raise();
        pushButton_alt->raise();
        lcdNumber_2->raise();
        pushButton_10->raise();
        tabWidget->raise();
        pushButton_norm->raise();
        select_gyro_page->raise();
        select_camera_from_transponder->raise();
        label_17->raise();
        label_3->raise();
        page = new QWidget();
        page->setObjectName("page");
        label_4 = new QLabel(page);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(10, 40, 91, 31));
        QFont font12;
        font12.setPointSize(20);
        font12.setItalic(true);
        label_4->setFont(font12);
        label_4->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        timeEdit = new QTimeEdit(page);
        timeEdit->setObjectName("timeEdit");
        timeEdit->setEnabled(false);
        timeEdit->setGeometry(QRect(10, 70, 191, 41));
        QFont font13;
        font13.setPointSize(25);
        timeEdit->setFont(font13);
        timeEdit->setReadOnly(true);
        timeEdit->setDateTime(QDateTime(QDate(2023, 12, 29), QTime(0, 0, 0)));
        timeEdit->setTimeSpec(Qt::TimeSpec::UTC);
        label_5 = new QLabel(page);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(10, 120, 131, 31));
        label_5->setFont(font12);
        label_5->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        timeEdit_2 = new QTimeEdit(page);
        timeEdit_2->setObjectName("timeEdit_2");
        timeEdit_2->setEnabled(false);
        timeEdit_2->setGeometry(QRect(10, 150, 191, 41));
        timeEdit_2->setFont(font13);
        timeEdit_2->setReadOnly(true);
        timeEdit_2->setTimeSpec(Qt::TimeSpec::LocalTime);
        timeEdit_2->setTime(QTime(0, 0, 0));
        label_6 = new QLabel(page);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(10, 0, 191, 41));
        QFont font14;
        font14.setPointSize(22);
        font14.setBold(true);
        font14.setItalic(true);
        label_6->setFont(font14);
        label_6->setStyleSheet(QString::fromUtf8("color: rgb(15,185,15);"));
        graphicsView = new QGraphicsView(page);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(220, 0, 451, 381));
        graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        compass = new QLabel(page);
        compass->setObjectName("compass");
        compass->setGeometry(QRect(230, 10, 101, 21));
        QFont font15;
        font15.setPointSize(26);
        font15.setBold(true);
        font15.setItalic(true);
        compass->setFont(font15);
        compass->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        compass->setAlignment(Qt::AlignmentFlag::AlignCenter);
        speed = new QLabel(page);
        speed->setObjectName("speed");
        speed->setGeometry(QRect(400, 10, 111, 21));
        speed->setFont(font15);
        speed->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        speed->setAlignment(Qt::AlignmentFlag::AlignCenter);
        roll = new QLabel(page);
        roll->setObjectName("roll");
        roll->setGeometry(QRect(230, 327, 81, 21));
        roll->setFont(font15);
        roll->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        roll->setAlignment(Qt::AlignmentFlag::AlignCenter);
        pitch = new QLabel(page);
        pitch->setObjectName("pitch");
        pitch->setGeometry(QRect(570, 327, 91, 21));
        pitch->setFont(font15);
        pitch->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        pitch->setAlignment(Qt::AlignmentFlag::AlignCenter);
        select_dumy_page2 = new QPushButton(page);
        select_dumy_page2->setObjectName("select_dumy_page2");
        select_dumy_page2->setGeometry(QRect(680, 10, 51, 161));
        select_dumy_page2->setFont(font9);
        select_dumy_page2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
"\n"
""));
        select_transponder_page = new QPushButton(page);
        select_transponder_page->setObjectName("select_transponder_page");
        select_transponder_page->setGeometry(QRect(680, 180, 51, 161));
        select_transponder_page->setFont(font9);
        select_transponder_page->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        timer_start = new QPushButton(page);
        timer_start->setObjectName("timer_start");
        timer_start->setGeometry(QRect(10, 270, 181, 71));
        timer_start->setFont(font5);
        timer_start->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        label_7 = new QLabel(page);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(240, 35, 81, 31));
        label_7->setFont(font12);
        label_7->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_7->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_8 = new QLabel(page);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(240, 300, 51, 21));
        label_8->setFont(font12);
        label_8->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_8->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_9 = new QLabel(page);
        label_9->setObjectName("label_9");
        label_9->setGeometry(QRect(590, 300, 61, 21));
        label_9->setFont(font12);
        label_9->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_9->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_10 = new QLabel(page);
        label_10->setObjectName("label_10");
        label_10->setGeometry(QRect(400, 40, 111, 21));
        label_10->setFont(font12);
        label_10->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_10->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_11 = new QLabel(page);
        label_11->setObjectName("label_11");
        label_11->setGeometry(QRect(560, 40, 101, 21));
        label_11->setFont(font12);
        label_11->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_11->setAlignment(Qt::AlignmentFlag::AlignCenter);
        altitude = new QLabel(page);
        altitude->setObjectName("altitude");
        altitude->setGeometry(QRect(570, 10, 91, 21));
        altitude->setFont(font15);
        altitude->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        altitude->setAlignment(Qt::AlignmentFlag::AlignCenter);
        temp = new QLabel(page);
        temp->setObjectName("temp");
        temp->setGeometry(QRect(400, 330, 111, 21));
        temp->setFont(font15);
        temp->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        temp->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_12 = new QLabel(page);
        label_12->setObjectName("label_12");
        label_12->setGeometry(QRect(390, 300, 121, 21));
        label_12->setFont(font12);
        label_12->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_12->setAlignment(Qt::AlignmentFlag::AlignCenter);
        listView = new QPlainTextEdit(page);
        listView->setObjectName("listView");
        listView->setGeometry(QRect(200, 0, 461, 391));
        QFont font16;
        font16.setFamilies({QString::fromUtf8("Academy Engraved LET")});
        listView->setFont(font16);
        listView->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        listView->setReadOnly(true);
        label_15 = new QLabel(page);
        label_15->setObjectName("label_15");
        label_15->setGeometry(QRect(230, 100, 101, 16));
        label_15->setFont(font10);
        label_15->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_15->setAlignment(Qt::AlignmentFlag::AlignCenter);
        temperature = new QLabel(page);
        temperature->setObjectName("temperature");
        temperature->setGeometry(QRect(240, 80, 81, 21));
        temperature->setFont(font14);
        temperature->setStyleSheet(QString::fromUtf8("color: rgb(138, 150, 155);"));
        temperature->setAlignment(Qt::AlignmentFlag::AlignCenter);
        baro_alt = new QLabel(page);
        baro_alt->setObjectName("baro_alt");
        baro_alt->setGeometry(QRect(570, 80, 91, 21));
        baro_alt->setFont(font14);
        baro_alt->setStyleSheet(QString::fromUtf8("color: rgb(138, 150, 155);"));
        baro_alt->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_16 = new QLabel(page);
        label_16->setObjectName("label_16");
        label_16->setGeometry(QRect(560, 100, 101, 21));
        label_16->setFont(font10);
        label_16->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_16->setAlignment(Qt::AlignmentFlag::AlignCenter);
        stackedWidget->addWidget(page);
        listView->raise();
        label_4->raise();
        timeEdit->raise();
        label_5->raise();
        timeEdit_2->raise();
        label_6->raise();
        graphicsView->raise();
        compass->raise();
        speed->raise();
        roll->raise();
        pitch->raise();
        select_dumy_page2->raise();
        select_transponder_page->raise();
        timer_start->raise();
        label_7->raise();
        label_8->raise();
        label_9->raise();
        label_10->raise();
        label_11->raise();
        altitude->raise();
        temp->raise();
        label_12->raise();
        label_15->raise();
        temperature->raise();
        baro_alt->raise();
        label_16->raise();
        page_6 = new QWidget();
        page_6->setObjectName("page_6");
        select_transponder_page_2 = new QPushButton(page_6);
        select_transponder_page_2->setObjectName("select_transponder_page_2");
        select_transponder_page_2->setGeometry(QRect(680, 180, 51, 161));
        select_transponder_page_2->setFont(font9);
        select_transponder_page_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_dumy_page2_2 = new QPushButton(page_6);
        select_dumy_page2_2->setObjectName("select_dumy_page2_2");
        select_dumy_page2_2->setGeometry(QRect(680, 10, 51, 161));
        select_dumy_page2_2->setFont(font9);
        select_dumy_page2_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        widgetai = new WidgetAI(page_6);
        widgetai->setObjectName("widgetai");
        widgetai->setGeometry(QRect(410, 100, 261, 261));
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(99);
        sizePolicy1.setVerticalStretch(99);
        sizePolicy1.setHeightForWidth(widgetai->sizePolicy().hasHeightForWidth());
        widgetai->setSizePolicy(sizePolicy1);
        widgetTC = new WidgetTC(page_6);
        widgetTC->setObjectName("widgetTC");
        widgetTC->setGeometry(QRect(240, 10, 191, 171));
        sizePolicy1.setHeightForWidth(widgetTC->sizePolicy().hasHeightForWidth());
        widgetTC->setSizePolicy(sizePolicy1);
        widgetALT = new WidgetALT(page_6);
        widgetALT->setObjectName("widgetALT");
        widgetALT->setGeometry(QRect(0, 110, 271, 251));
        sizePolicy1.setHeightForWidth(widgetALT->sizePolicy().hasHeightForWidth());
        widgetALT->setSizePolicy(sizePolicy1);
        widgetASI = new WidgetASI(page_6);
        widgetASI->setObjectName("widgetASI");
        widgetASI->setGeometry(QRect(240, 180, 181, 181));
        sizePolicy1.setHeightForWidth(widgetASI->sizePolicy().hasHeightForWidth());
        widgetASI->setSizePolicy(sizePolicy1);
        widgetVSI = new WidgetVSI(page_6);
        widgetVSI->setObjectName("widgetVSI");
        widgetVSI->setGeometry(QRect(0, 0, 108, 108));
        sizePolicy1.setHeightForWidth(widgetVSI->sizePolicy().hasHeightForWidth());
        widgetVSI->setSizePolicy(sizePolicy1);
        widgetHI = new WidgetHI(page_6);
        widgetHI->setObjectName("widgetHI");
        widgetHI->setGeometry(QRect(570, 1, 108, 108));
        sizePolicy1.setHeightForWidth(widgetHI->sizePolicy().hasHeightForWidth());
        widgetHI->setSizePolicy(sizePolicy1);
        radioButton = new QRadioButton(page_6);
        radioButton->setObjectName("radioButton");
        radioButton->setGeometry(QRect(440, 10, 21, 20));
        radioButton->setStyleSheet(QString::fromUtf8("QRadioButton::indicator:checked{\n"
"width:12px;height:12px;\n"
"border-radius:7px;\n"
"background-color:red;\n"
"border:2px solid white;\n"
"color: white;\n"
"}\n"
"QRadioButton::indicator:unchecked{\n"
"width:10px;height:10px;\n"
"border-radius:5px;\n"
"background-color: blue;\n"
"border:2px solid transparent;\n"
"color: rgb(255, 255, 255);\n"
"}"));
        radioButton_2 = new QRadioButton(page_6);
        radioButton_2->setObjectName("radioButton_2");
        radioButton_2->setGeometry(QRect(440, 30, 21, 20));
        radioButton_2->setStyleSheet(QString::fromUtf8("QRadioButton::indicator:checked{\n"
"width:12px;height:12px;\n"
"border-radius:7px;\n"
"background-color:red;\n"
"border:2px solid white;\n"
"color: white;\n"
"}\n"
"QRadioButton::indicator:unchecked{\n"
"width:10px;height:10px;\n"
"border-radius:5px;\n"
"background-color: blue;\n"
"border:2px solid transparent;\n"
"color: rgb(255, 255, 255);\n"
"}\n"
"\n"
"\n"
""));
        radioButton_2->setChecked(true);
        radioButton_3 = new QRadioButton(page_6);
        radioButton_3->setObjectName("radioButton_3");
        radioButton_3->setGeometry(QRect(440, 50, 21, 20));
        radioButton_3->setStyleSheet(QString::fromUtf8("QRadioButton::indicator:checked{\n"
"width:12px;height:12px;\n"
"border-radius:7px;\n"
"background-color:red;\n"
"border:2px solid white;\n"
"color: white;\n"
"}\n"
"QRadioButton::indicator:unchecked{\n"
"width:10px;height:10px;\n"
"border-radius:5px;\n"
"background-color: blue;\n"
"border:2px solid transparent;\n"
"color: rgb(255, 255, 255);\n"
"}"));
        radioButton_3->setChecked(false);
        label_18 = new QLabel(page_6);
        label_18->setObjectName("label_18");
        label_18->setGeometry(QRect(470, 50, 101, 21));
        label_18->setFont(font10);
        label_18->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_18->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        label_19 = new QLabel(page_6);
        label_19->setObjectName("label_19");
        label_19->setGeometry(QRect(470, 30, 101, 21));
        label_19->setFont(font10);
        label_19->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_19->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        label_20 = new QLabel(page_6);
        label_20->setObjectName("label_20");
        label_20->setGeometry(QRect(470, 10, 101, 21));
        label_20->setFont(font10);
        label_20->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_20->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        dial = new QDial(page_6);
        dial->setObjectName("dial");
        dial->setGeometry(QRect(160, 40, 71, 71));
        dial->setMinimum(94000);
        dial->setMaximum(105000);
        dial->setSingleStep(10);
        dial->setPageStep(100);
        dial->setValue(101325);
        dial->setSliderPosition(101325);
        dial->setOrientation(Qt::Orientation::Horizontal);
        dial->setInvertedAppearance(false);
        dial->setWrapping(true);
        dial->setNotchTarget(15.000000000000000);
        dial->setNotchesVisible(true);
        doubleSpinBox = new QLineEdit(page_6);
        doubleSpinBox->setObjectName("doubleSpinBox");
        doubleSpinBox->setGeometry(QRect(160, 10, 71, 21));
        QFont font17;
        font17.setFamilies({QString::fromUtf8("Arial")});
        font17.setPointSize(16);
        doubleSpinBox->setFont(font17);
        doubleSpinBox->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        doubleSpinBox->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);
        stackedWidget->addWidget(page_6);
        widgetTC->raise();
        select_transponder_page_2->raise();
        widgetai->raise();
        widgetALT->raise();
        widgetASI->raise();
        widgetVSI->raise();
        widgetHI->raise();
        radioButton->raise();
        radioButton_2->raise();
        radioButton_3->raise();
        label_18->raise();
        label_19->raise();
        label_20->raise();
        dial->raise();
        doubleSpinBox->raise();
        select_dumy_page2_2->raise();
        page_7 = new QWidget();
        page_7->setObjectName("page_7");
        select_transponder_page_3 = new QPushButton(page_7);
        select_transponder_page_3->setObjectName("select_transponder_page_3");
        select_transponder_page_3->setGeometry(QRect(680, 180, 51, 161));
        select_transponder_page_3->setFont(font9);
        select_transponder_page_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_from_4_to_5 = new QPushButton(page_7);
        select_from_4_to_5->setObjectName("select_from_4_to_5");
        select_from_4_to_5->setGeometry(QRect(680, 10, 51, 161));
        select_from_4_to_5->setFont(font9);
        select_from_4_to_5->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        widgetEADI = new WidgetEADI(page_7);
        widgetEADI->setObjectName("widgetEADI");
        widgetEADI->setGeometry(QRect(0, 0, 331, 361));
        widgetEHSI = new WidgetEHSI(page_7);
        widgetEHSI->setObjectName("widgetEHSI");
        widgetEHSI->setGeometry(QRect(340, -1, 331, 361));
        dial_2 = new QDial(page_7);
        dial_2->setObjectName("dial_2");
        dial_2->setGeometry(QRect(340, 290, 71, 71));
        dial_2->setMinimum(94000);
        dial_2->setMaximum(105000);
        dial_2->setSingleStep(10);
        dial_2->setPageStep(100);
        dial_2->setValue(101325);
        dial_2->setSliderPosition(101325);
        dial_2->setOrientation(Qt::Orientation::Horizontal);
        dial_2->setInvertedAppearance(false);
        dial_2->setWrapping(true);
        dial_2->setNotchTarget(15.000000000000000);
        dial_2->setNotchesVisible(true);
        doubleSpinBox_2 = new QLineEdit(page_7);
        doubleSpinBox_2->setObjectName("doubleSpinBox_2");
        doubleSpinBox_2->setGeometry(QRect(340, 260, 71, 21));
        doubleSpinBox_2->setFont(font17);
        doubleSpinBox_2->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        doubleSpinBox_2->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);
        stackedWidget->addWidget(page_7);
        page_9 = new QWidget();
        page_9->setObjectName("page_9");
        select_transponder_page_4 = new QPushButton(page_9);
        select_transponder_page_4->setObjectName("select_transponder_page_4");
        select_transponder_page_4->setGeometry(QRect(680, 180, 51, 161));
        select_transponder_page_4->setFont(font9);
        select_transponder_page_4->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_from_4_to_6 = new QPushButton(page_9);
        select_from_4_to_6->setObjectName("select_from_4_to_6");
        select_from_4_to_6->setGeometry(QRect(680, 10, 51, 161));
        select_from_4_to_6->setFont(font9);
        select_from_4_to_6->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        label_22 = new QLabel(page_9);
        label_22->setObjectName("label_22");
        label_22->setGeometry(QRect(10, 20, 111, 21));
        QFont font18;
        font18.setFamilies({QString::fromUtf8("Academy Engraved LET")});
        font18.setPointSize(20);
        label_22->setFont(font18);
        label_22->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);\n"
""));
        radar_speed = new QLabel(page_9);
        radar_speed->setObjectName("radar_speed");
        radar_speed->setGeometry(QRect(570, 30, 91, 21));
        radar_speed->setFont(font14);
        radar_speed->setStyleSheet(QString::fromUtf8("color: rgb(138, 150, 155);"));
        radar_speed->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_27 = new QLabel(page_9);
        label_27->setObjectName("label_27");
        label_27->setGeometry(QRect(10, 190, 31, 16));
        label_27->setStyleSheet(QString::fromUtf8("color: rgb(255, 172, 26);"));
        label_26 = new QLabel(page_9);
        label_26->setObjectName("label_26");
        label_26->setGeometry(QRect(10, 40, 41, 16));
        label_26->setStyleSheet(QString::fromUtf8("color: rgb(255, 56, 25);"));
        graphicsView_2 = new QGraphicsView(page_9);
        graphicsView_2->setObjectName("graphicsView_2");
        graphicsView_2->setGeometry(QRect(0, 10, 671, 351));
        graphicsView_2->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsView_2->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        label_28 = new QLabel(page_9);
        label_28->setObjectName("label_28");
        label_28->setGeometry(QRect(10, 340, 21, 16));
        label_28->setStyleSheet(QString::fromUtf8("color: rgb(26, 255, 29);"));
        label_29 = new QLabel(page_9);
        label_29->setObjectName("label_29");
        label_29->setGeometry(QRect(560, 50, 101, 21));
        label_29->setFont(font10);
        label_29->setStyleSheet(QString::fromUtf8("color: rgb(135,135,135);"));
        label_29->setAlignment(Qt::AlignmentFlag::AlignCenter);
        stackedWidget->addWidget(page_9);
        graphicsView_2->raise();
        select_transponder_page_4->raise();
        select_from_4_to_6->raise();
        label_22->raise();
        radar_speed->raise();
        label_27->raise();
        label_26->raise();
        label_28->raise();
        label_29->raise();
        page_3 = new QWidget();
        page_3->setObjectName("page_3");
        select_transponder_page2 = new QPushButton(page_3);
        select_transponder_page2->setObjectName("select_transponder_page2");
        select_transponder_page2->setGeometry(QRect(680, 10, 51, 161));
        select_transponder_page2->setFont(font9);
        select_transponder_page2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_gyro_page2 = new QPushButton(page_3);
        select_gyro_page2->setObjectName("select_gyro_page2");
        select_gyro_page2->setGeometry(QRect(680, 180, 51, 161));
        select_gyro_page2->setFont(font9);
        select_gyro_page2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        textEdit = new QTextEdit(page_3);
        textEdit->setObjectName("textEdit");
        textEdit->setGeometry(QRect(10, 40, 251, 311));
        QFont font19;
        font19.setFamilies({QString::fromUtf8("Courier New")});
        font19.setPointSize(18);
        textEdit->setFont(font19);
        textEdit->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        textEdit_2 = new QTextEdit(page_3);
        textEdit_2->setObjectName("textEdit_2");
        textEdit_2->setGeometry(QRect(270, 40, 401, 311));
        textEdit_2->setFont(font19);
        textEdit_2->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        textEdit_2->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        label_13 = new QLabel(page_3);
        label_13->setObjectName("label_13");
        label_13->setGeometry(QRect(20, 10, 151, 21));
        label_13->setFont(font12);
        label_13->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_14 = new QLabel(page_3);
        label_14->setObjectName("label_14");
        label_14->setGeometry(QRect(320, 10, 151, 21));
        label_14->setFont(font12);
        label_14->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        pushButton_15 = new QPushButton(page_3);
        pushButton_15->setObjectName("pushButton_15");
        pushButton_15->setGeometry(QRect(500, 10, 151, 31));
        pushButton_15->setFont(font);
        pushButton_15->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(255, 166, 20);\n"
"border: 2px solid #555;\n"
"border-radius: 10px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #040\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
"\n"
""));
        stackedWidget->addWidget(page_3);
        page_4 = new QWidget();
        page_4->setObjectName("page_4");
        select_transponder_page2_2 = new QPushButton(page_4);
        select_transponder_page2_2->setObjectName("select_transponder_page2_2");
        select_transponder_page2_2->setGeometry(QRect(680, 10, 51, 161));
        select_transponder_page2_2->setFont(font9);
        select_transponder_page2_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_gyro_page2_2 = new QPushButton(page_4);
        select_gyro_page2_2->setObjectName("select_gyro_page2_2");
        select_gyro_page2_2->setGeometry(QRect(680, 180, 51, 161));
        select_gyro_page2_2->setFont(font9);
        select_gyro_page2_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        fly_home = new QPushButton(page_4);
        fly_home->setObjectName("fly_home");
        fly_home->setGeometry(QRect(10, 320, 221, 31));
        fly_home->setFont(font9);
        fly_home->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        stackedWidget->addWidget(page_4);
        page_8 = new QWidget();
        page_8->setObjectName("page_8");
        select_transponder_page_camera = new QPushButton(page_8);
        select_transponder_page_camera->setObjectName("select_transponder_page_camera");
        select_transponder_page_camera->setGeometry(QRect(680, 10, 51, 161));
        select_transponder_page_camera->setFont(font9);
        select_transponder_page_camera->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_page2_map = new QPushButton(page_8);
        select_page2_map->setObjectName("select_page2_map");
        select_page2_map->setGeometry(QRect(680, 180, 51, 161));
        select_page2_map->setFont(font9);
        select_page2_map->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        reset_att = new QPushButton(page_8);
        reset_att->setObjectName("reset_att");
        reset_att->setGeometry(QRect(20, 160, 131, 51));
        reset_att->setFont(font5);
        reset_att->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #00F\n"
");\n"
"padding: 5px;\n"
"}\n"
""));
        reset_heading = new QPushButton(page_8);
        reset_heading->setObjectName("reset_heading");
        reset_heading->setGeometry(QRect(20, 230, 131, 51));
        reset_heading->setFont(font5);
        reset_heading->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        imu_reset = new QPushButton(page_8);
        imu_reset->setObjectName("imu_reset");
        imu_reset->setGeometry(QRect(20, 90, 131, 51));
        imu_reset->setFont(font5);
        imu_reset->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/config.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        imu_reset->setAutoDefault(false);
        use_hw = new QPushButton(page_8);
        use_hw->setObjectName("use_hw");
        use_hw->setGeometry(QRect(20, 20, 131, 51));
        use_hw->setFont(font5);
        use_hw->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/podcast.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}"));
        exit_2 = new QPushButton(page_8);
        exit_2->setObjectName("exit_2");
        exit_2->setGeometry(QRect(20, 300, 131, 51));
        QFont font20;
        font20.setPointSize(18);
        font20.setBold(true);
        exit_2->setFont(font20);
        exit_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        exit_2->setAutoDefault(false);
        use_gps_in_attitude = new QPushButton(page_8);
        use_gps_in_attitude->setObjectName("use_gps_in_attitude");
        use_gps_in_attitude->setGeometry(QRect(170, 160, 151, 51));
        use_gps_in_attitude->setFont(font5);
        use_gps_in_attitude->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        label = new QLabel(page_8);
        label->setObjectName("label");
        label->setGeometry(QRect(170, 20, 161, 41));
        QFont font21;
        font21.setPointSize(16);
        label->setFont(font21);
        label->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_21 = new QLabel(page_8);
        label_21->setObjectName("label_21");
        label_21->setGeometry(QRect(170, 90, 161, 41));
        label_21->setFont(font21);
        label_21->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_23 = new QLabel(page_8);
        label_23->setObjectName("label_23");
        label_23->setGeometry(QRect(340, 230, 331, 55));
        label_23->setFont(font21);
        label_23->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        reset_altitude_2 = new QPushButton(page_8);
        reset_altitude_2->setObjectName("reset_altitude_2");
        reset_altitude_2->setGeometry(QRect(170, 230, 151, 51));
        reset_altitude_2->setFont(font5);
        reset_altitude_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        reset_heading_3 = new QPushButton(page_8);
        reset_heading_3->setObjectName("reset_heading_3");
        reset_heading_3->setGeometry(QRect(170, 300, 151, 51));
        reset_heading_3->setFont(font5);
        reset_heading_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        label_24 = new QLabel(page_8);
        label_24->setObjectName("label_24");
        label_24->setGeometry(QRect(340, 290, 331, 55));
        label_24->setFont(font21);
        label_24->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        use_built_inn_barometer = new QPushButton(page_8);
        use_built_inn_barometer->setObjectName("use_built_inn_barometer");
        use_built_inn_barometer->setGeometry(QRect(370, 20, 151, 51));
        use_built_inn_barometer->setFont(font5);
        use_built_inn_barometer->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 15px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #080\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        stackedWidget->addWidget(page_8);
        page_5 = new QWidget();
        page_5->setObjectName("page_5");
        select_transponder_page2_3 = new QPushButton(page_5);
        select_transponder_page2_3->setObjectName("select_transponder_page2_3");
        select_transponder_page2_3->setGeometry(QRect(680, 10, 51, 161));
        select_transponder_page2_3->setFont(font9);
        select_transponder_page2_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/forwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_gyro_page2_3 = new QPushButton(page_5);
        select_gyro_page2_3->setObjectName("select_gyro_page2_3");
        select_gyro_page2_3->setGeometry(QRect(680, 180, 51, 161));
        select_gyro_page2_3->setFont(font9);
        select_gyro_page2_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"image: url(:/backwards.svg); \n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #15E\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        viewfinder = new QVideoWidget(page_5);
        viewfinder->setObjectName("viewfinder");
        viewfinder->setGeometry(QRect(70, 0, 591, 351));
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(viewfinder->sizePolicy().hasHeightForWidth());
        viewfinder->setSizePolicy(sizePolicy2);
        pushButton_20 = new QPushButton(page_5);
        pushButton_20->setObjectName("pushButton_20");
        pushButton_20->setGeometry(QRect(10, 10, 51, 71));
        pushButton_20->setFont(font9);
        pushButton_20->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #660\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        pushButton_21 = new QPushButton(page_5);
        pushButton_21->setObjectName("pushButton_21");
        pushButton_21->setGeometry(QRect(10, 90, 51, 71));
        pushButton_21->setFont(font9);
        pushButton_21->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #660\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        pushButton_22 = new QPushButton(page_5);
        pushButton_22->setObjectName("pushButton_22");
        pushButton_22->setGeometry(QRect(10, 170, 51, 71));
        pushButton_22->setFont(font9);
        pushButton_22->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #660\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        pushButton_23 = new QPushButton(page_5);
        pushButton_23->setObjectName("pushButton_23");
        pushButton_23->setGeometry(QRect(10, 250, 51, 91));
        pushButton_23->setFont(font9);
        pushButton_23->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #590\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        stackedWidget->addWidget(page_5);
        viewfinder->raise();
        select_transponder_page2_3->raise();
        select_gyro_page2_3->raise();
        pushButton_20->raise();
        pushButton_21->raise();
        pushButton_22->raise();
        pushButton_23->raise();
        MainWindow_port_iPhone->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow_port_iPhone);
        statusbar->setObjectName("statusbar");
        statusbar->setSizeGripEnabled(true);
        MainWindow_port_iPhone->setStatusBar(statusbar);

        retranslateUi(MainWindow_port_iPhone);

        stackedWidget->setCurrentIndex(4);
        tabWidget->setCurrentIndex(0);
        imu_reset->setDefault(true);
        exit_2->setDefault(true);


        QMetaObject::connectSlotsByName(MainWindow_port_iPhone);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow_port_iPhone)
    {
        MainWindow_port_iPhone->setWindowTitle(QCoreApplication::translate("MainWindow_port_iPhone", "MainWindow", nullptr));
        pushButton->setText(QString());
        pushButton_2->setText(QString());
        pushButton_3->setText(QString());
        pushButton_4->setText(QString());
        pushButton_5->setText(QString());
        pushButton_6->setText(QString());
        pushButton_7->setText(QString());
        pushButton_8->setText(QString());
        pushButton_9->setText(QString());
        pushButton_16->setText(QString());
        pushButton_17->setText(QString());
        pushButton_18->setText(QString());
        pushButton_14->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Com. OK", nullptr));
        pushButton_11->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Alt. OK", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Alt. ft.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow_port_iPhone", "Alt", nullptr));
        plainTextEdit->setPlainText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow_port_iPhone", "Status", nullptr));
        pushButton_19->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Exit", nullptr));
        pushButton_off->setText(QCoreApplication::translate("MainWindow_port_iPhone", "HW check", nullptr));
        pushButton_12->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Meter", nullptr));
        pushButton_13->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Feet", nullptr));
        reconnect_now->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Reconnect", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("MainWindow_port_iPhone", "Config", nullptr));
        pushButton_Ident->setText(QCoreApplication::translate("MainWindow_port_iPhone", "IDENT", nullptr));
        pushButton_stby->setText(QCoreApplication::translate("MainWindow_port_iPhone", "STBY", nullptr));
        pushButton_norm->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Norm", nullptr));
        pushButton_alt->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Alt", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Ping OK", nullptr));
        select_gyro_page->setText(QString());
        select_camera_from_transponder->setText(QString());
        label_17->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Squawk", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow_port_iPhone", "v1.01a-2024", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow_port_iPhone", "UTC Time:", nullptr));
        timeEdit->setDisplayFormat(QCoreApplication::translate("MainWindow_port_iPhone", "dd/MM HH:mm.ss", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Flight timer:", nullptr));
        timeEdit_2->setDisplayFormat(QCoreApplication::translate("MainWindow_port_iPhone", "HH:mm:ss", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Artificial Horizon:", nullptr));
        compass->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        speed->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        roll->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        pitch->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        select_dumy_page2->setText(QString());
        select_transponder_page->setText(QString());
        timer_start->setText(QCoreApplication::translate("MainWindow_port_iPhone", "ViewLogFile", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Bearing", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Roll", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Pitch", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Speed km/h", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow_port_iPhone", "GPS Alt. ft.", nullptr));
        altitude->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        temp->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow_port_iPhone", "IMU Power", nullptr));
        label_15->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Temperature", nullptr));
        temperature->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        baro_alt->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        label_16->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Baro. Alt.", nullptr));
        select_transponder_page_2->setText(QString());
        select_dumy_page2_2->setText(QString());
        radioButton->setText(QString());
        radioButton_2->setText(QString());
        radioButton_3->setText(QString());
        label_18->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Baro", nullptr));
        label_19->setText(QCoreApplication::translate("MainWindow_port_iPhone", "GPS", nullptr));
        label_20->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Transponder", nullptr));
        doubleSpinBox->setText(QCoreApplication::translate("MainWindow_port_iPhone", "1013.25", nullptr));
        select_transponder_page_3->setText(QString());
        select_from_4_to_5->setText(QString());
        doubleSpinBox_2->setText(QCoreApplication::translate("MainWindow_port_iPhone", "1013.25", nullptr));
        select_transponder_page_4->setText(QString());
        select_from_4_to_6->setText(QString());
        label_22->setText(QCoreApplication::translate("MainWindow_port_iPhone", "RadarView", nullptr));
        radar_speed->setText(QCoreApplication::translate("MainWindow_port_iPhone", "--", nullptr));
        label_27->setText(QCoreApplication::translate("MainWindow_port_iPhone", "50 M", nullptr));
        label_26->setText(QCoreApplication::translate("MainWindow_port_iPhone", "100 M", nullptr));
        label_28->setText(QCoreApplication::translate("MainWindow_port_iPhone", "0 M", nullptr));
        label_29->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Radar Speed", nullptr));
        select_transponder_page2->setText(QString());
        select_gyro_page2->setText(QString());
        textEdit->setHtml(QCoreApplication::translate("MainWindow_port_iPhone", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Courier New'; font-size:18pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'.AppleSystemUIFont'; font-size:13pt;\"><br /></p></body></html>", nullptr));
        textEdit_2->setHtml(QCoreApplication::translate("MainWindow_port_iPhone", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Courier New'; font-size:18pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'.AppleSystemUIFont'; font-size:13pt;\"><br /></p></body></html>", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Radio frequencies:", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Airplanes:", nullptr));
        pushButton_15->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Reset List", nullptr));
        select_transponder_page2_2->setText(QString());
        select_gyro_page2_2->setText(QString());
        fly_home->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Fly Home", nullptr));
        select_transponder_page_camera->setText(QString());
        select_page2_map->setText(QString());
        reset_att->setText(QCoreApplication::translate("MainWindow_port_iPhone", "EKF_1", nullptr));
        reset_heading->setText(QCoreApplication::translate("MainWindow_port_iPhone", "North", nullptr));
        imu_reset->setText(QString());
        use_hw->setText(QString());
        exit_2->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Exit", nullptr));
        use_gps_in_attitude->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Use GPS in algo.", nullptr));
        label->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Full recalibrate", nullptr));
        label_21->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Pitch recalibrate", nullptr));
        label_23->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Calibrate Attitude in Barometer from GPS\n"
"Only to be used on ground !", nullptr));
        reset_altitude_2->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Calibrate Altitude", nullptr));
        reset_heading_3->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Calibrate Heading", nullptr));
        label_24->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Calibrate Heading in compass from GPS\n"
"Only to be used in air !", nullptr));
        use_built_inn_barometer->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Use External\n"
"barometer", nullptr));
        select_transponder_page2_3->setText(QString());
        select_gyro_page2_3->setText(QString());
        pushButton_20->setText(QCoreApplication::translate("MainWindow_port_iPhone", "P", nullptr));
        pushButton_21->setText(QCoreApplication::translate("MainWindow_port_iPhone", "3s", nullptr));
        pushButton_22->setText(QCoreApplication::translate("MainWindow_port_iPhone", "10s", nullptr));
        pushButton_23->setText(QCoreApplication::translate("MainWindow_port_iPhone", "Stop", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow_port_iPhone: public Ui_MainWindow_port_iPhone {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_PORT_IPHONE_H
