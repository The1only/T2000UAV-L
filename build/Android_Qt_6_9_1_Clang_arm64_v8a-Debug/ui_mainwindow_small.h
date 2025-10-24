/********************************************************************************
** Form generated from reading UI file 'mainwindow_small.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_SMALL_H
#define UI_MAINWINDOW_SMALL_H

#include <QtCore/QVariant>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtQuickWidgets/QQuickWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow_small
{
public:
    QWidget *centralwidget;
    QFrame *frame_2;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QLabel *label_4;
    QTimeEdit *timeEdit;
    QLabel *label_5;
    QTimeEdit *timeEdit_2;
    QGraphicsView *graphicsView;
    QLabel *compass;
    QLabel *speed;
    QLabel *roll;
    QLabel *pitch;
    QPushButton *select_dumy_page2;
    QPushButton *select_transponder_page;
    QPushButton *timer_start;
    QPushButton *imu_reset;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *altitude;
    QLabel *temp;
    QLabel *label_12;
    QPlainTextEdit *listView;
    QPushButton *exit_2;
    QLabel *label_6;
    QLabel *label_15;
    QLabel *temperature;
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
    QQuickWidget *quickWidget;
    QWidget *page_5;
    QVideoWidget *viewfinder;
    QPushButton *pushButton_20;
    QPushButton *select_transponder_page2_3;
    QPushButton *select_gyro_page2_3;
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
    QLCDNumber *lcdNumber;
    QPushButton *pushButton_Ident;
    QPushButton *pushButton_stby;
    QPushButton *pushButton_norm;
    QPushButton *pushButton_alt;
    QLCDNumber *lcdNumber_2;
    QPushButton *pushButton_10;
    QPushButton *select_gyro_page;
    QPushButton *select_dumy_page;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow_small)
    {
        if (MainWindow_small->objectName().isEmpty())
            MainWindow_small->setObjectName("MainWindow_small");
        MainWindow_small->resize(681, 340);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow_small->sizePolicy().hasHeightForWidth());
        MainWindow_small->setSizePolicy(sizePolicy);
        MainWindow_small->setMinimumSize(QSize(681, 340));
        MainWindow_small->setMaximumSize(QSize(681, 340));
        MainWindow_small->setStyleSheet(QString::fromUtf8("background-color: rgb(0,0,0);"));
        MainWindow_small->setUnifiedTitleAndToolBarOnMac(false);
        centralwidget = new QWidget(MainWindow_small);
        centralwidget->setObjectName("centralwidget");
        frame_2 = new QFrame(centralwidget);
        frame_2->setObjectName("frame_2");
        frame_2->setGeometry(QRect(10, 360, 900, 91));
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Raised);
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(0, 0, 681, 311));
        stackedWidget->setStyleSheet(QString::fromUtf8("\n"
"color: rgb(255, 255, 255);"));
        page = new QWidget();
        page->setObjectName("page");
        label_4 = new QLabel(page);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(20, 40, 91, 31));
        QFont font;
        font.setPointSize(20);
        font.setItalic(true);
        label_4->setFont(font);
        label_4->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        timeEdit = new QTimeEdit(page);
        timeEdit->setObjectName("timeEdit");
        timeEdit->setEnabled(false);
        timeEdit->setGeometry(QRect(20, 70, 171, 41));
        QFont font1;
        font1.setPointSize(25);
        timeEdit->setFont(font1);
        timeEdit->setReadOnly(true);
        timeEdit->setDateTime(QDateTime(QDate(2024, 1, 1), QTime(1, 0, 0)));
        label_5 = new QLabel(page);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(20, 120, 131, 31));
        label_5->setFont(font);
        label_5->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        timeEdit_2 = new QTimeEdit(page);
        timeEdit_2->setObjectName("timeEdit_2");
        timeEdit_2->setEnabled(false);
        timeEdit_2->setGeometry(QRect(20, 150, 171, 41));
        timeEdit_2->setFont(font1);
        graphicsView = new QGraphicsView(page);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(220, 0, 371, 311));
        graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        compass = new QLabel(page);
        compass->setObjectName("compass");
        compass->setGeometry(QRect(230, 10, 91, 21));
        QFont font2;
        font2.setPointSize(26);
        font2.setBold(true);
        font2.setItalic(true);
        compass->setFont(font2);
        compass->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        compass->setAlignment(Qt::AlignmentFlag::AlignCenter);
        speed = new QLabel(page);
        speed->setObjectName("speed");
        speed->setGeometry(QRect(490, 10, 91, 21));
        speed->setFont(font2);
        speed->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        speed->setAlignment(Qt::AlignmentFlag::AlignCenter);
        roll = new QLabel(page);
        roll->setObjectName("roll");
        roll->setGeometry(QRect(230, 280, 91, 21));
        roll->setFont(font2);
        roll->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        roll->setAlignment(Qt::AlignmentFlag::AlignCenter);
        pitch = new QLabel(page);
        pitch->setObjectName("pitch");
        pitch->setGeometry(QRect(490, 280, 91, 21));
        pitch->setFont(font2);
        pitch->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        pitch->setAlignment(Qt::AlignmentFlag::AlignCenter);
        select_dumy_page2 = new QPushButton(page);
        select_dumy_page2->setObjectName("select_dumy_page2");
        select_dumy_page2->setGeometry(QRect(620, 10, 51, 141));
        select_dumy_page2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_transponder_page = new QPushButton(page);
        select_transponder_page->setObjectName("select_transponder_page");
        select_transponder_page->setGeometry(QRect(620, 170, 51, 141));
        select_transponder_page->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
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
        timer_start->setGeometry(QRect(20, 200, 151, 31));
        QFont font3;
        font3.setPointSize(18);
        font3.setBold(true);
        timer_start->setFont(font3);
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
        imu_reset = new QPushButton(page);
        imu_reset->setObjectName("imu_reset");
        imu_reset->setGeometry(QRect(20, 240, 151, 31));
        imu_reset->setFont(font3);
        imu_reset->setStyleSheet(QString::fromUtf8("QPushButton {\n"
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
        label_7 = new QLabel(page);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(240, 40, 81, 21));
        label_7->setFont(font);
        label_7->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_7->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_8 = new QLabel(page);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(250, 250, 51, 21));
        label_8->setFont(font);
        label_8->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_8->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_9 = new QLabel(page);
        label_9->setObjectName("label_9");
        label_9->setGeometry(QRect(510, 250, 61, 21));
        label_9->setFont(font);
        label_9->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_9->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_10 = new QLabel(page);
        label_10->setObjectName("label_10");
        label_10->setGeometry(QRect(470, 40, 111, 21));
        label_10->setFont(font);
        label_10->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_10->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_11 = new QLabel(page);
        label_11->setObjectName("label_11");
        label_11->setGeometry(QRect(360, 40, 81, 21));
        label_11->setFont(font);
        label_11->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_11->setAlignment(Qt::AlignmentFlag::AlignCenter);
        altitude = new QLabel(page);
        altitude->setObjectName("altitude");
        altitude->setGeometry(QRect(360, 10, 91, 21));
        altitude->setFont(font2);
        altitude->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        altitude->setAlignment(Qt::AlignmentFlag::AlignCenter);
        temp = new QLabel(page);
        temp->setObjectName("temp");
        temp->setGeometry(QRect(360, 280, 91, 21));
        temp->setFont(font2);
        temp->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        temp->setAlignment(Qt::AlignmentFlag::AlignCenter);
        label_12 = new QLabel(page);
        label_12->setObjectName("label_12");
        label_12->setGeometry(QRect(370, 250, 71, 21));
        label_12->setFont(font);
        label_12->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_12->setAlignment(Qt::AlignmentFlag::AlignCenter);
        listView = new QPlainTextEdit(page);
        listView->setObjectName("listView");
        listView->setGeometry(QRect(200, 0, 401, 311));
        exit_2 = new QPushButton(page);
        exit_2->setObjectName("exit_2");
        exit_2->setGeometry(QRect(20, 280, 151, 31));
        exit_2->setFont(font3);
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
        label_6 = new QLabel(page);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(20, 10, 171, 21));
        QFont font4;
        font4.setPointSize(22);
        font4.setItalic(true);
        label_6->setFont(font4);
        label_6->setStyleSheet(QString::fromUtf8("color: rgb(15,185,15);"));
        label_15 = new QLabel(page);
        label_15->setObjectName("label_15");
        label_15->setGeometry(QRect(230, 90, 91, 31));
        QFont font5;
        font5.setPointSize(15);
        font5.setItalic(true);
        label_15->setFont(font5);
        label_15->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_15->setAlignment(Qt::AlignmentFlag::AlignCenter);
        temperature = new QLabel(page);
        temperature->setObjectName("temperature");
        temperature->setGeometry(QRect(230, 70, 81, 21));
        QFont font6;
        font6.setPointSize(22);
        font6.setBold(true);
        font6.setItalic(true);
        temperature->setFont(font6);
        temperature->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        temperature->setAlignment(Qt::AlignmentFlag::AlignCenter);
        stackedWidget->addWidget(page);
        listView->raise();
        label_4->raise();
        timeEdit->raise();
        label_5->raise();
        timeEdit_2->raise();
        graphicsView->raise();
        compass->raise();
        speed->raise();
        roll->raise();
        pitch->raise();
        select_dumy_page2->raise();
        select_transponder_page->raise();
        timer_start->raise();
        imu_reset->raise();
        label_7->raise();
        label_8->raise();
        label_9->raise();
        label_10->raise();
        label_11->raise();
        altitude->raise();
        temp->raise();
        label_12->raise();
        exit_2->raise();
        label_6->raise();
        label_15->raise();
        temperature->raise();
        page_3 = new QWidget();
        page_3->setObjectName("page_3");
        select_transponder_page2 = new QPushButton(page_3);
        select_transponder_page2->setObjectName("select_transponder_page2");
        select_transponder_page2->setGeometry(QRect(620, 10, 51, 141));
        select_transponder_page2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
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
        select_gyro_page2->setGeometry(QRect(620, 170, 51, 141));
        select_gyro_page2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
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
        textEdit->setGeometry(QRect(10, 40, 241, 271));
        QFont font7;
        font7.setPointSize(18);
        textEdit->setFont(font7);
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        textEdit_2 = new QTextEdit(page_3);
        textEdit_2->setObjectName("textEdit_2");
        textEdit_2->setGeometry(QRect(260, 40, 341, 271));
        textEdit_2->setFont(font7);
        textEdit_2->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        label_13 = new QLabel(page_3);
        label_13->setObjectName("label_13");
        label_13->setGeometry(QRect(20, 10, 151, 21));
        label_13->setFont(font);
        label_13->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        label_14 = new QLabel(page_3);
        label_14->setObjectName("label_14");
        label_14->setGeometry(QRect(270, 10, 151, 21));
        label_14->setFont(font);
        label_14->setStyleSheet(QString::fromUtf8("color: rgb(155,155,155);"));
        pushButton_15 = new QPushButton(page_3);
        pushButton_15->setObjectName("pushButton_15");
        pushButton_15->setGeometry(QRect(460, 10, 100, 31));
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
""));
        stackedWidget->addWidget(page_3);
        page_4 = new QWidget();
        page_4->setObjectName("page_4");
        select_transponder_page2_2 = new QPushButton(page_4);
        select_transponder_page2_2->setObjectName("select_transponder_page2_2");
        select_transponder_page2_2->setGeometry(QRect(620, 10, 51, 141));
        select_transponder_page2_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
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
        select_gyro_page2_2->setGeometry(QRect(620, 170, 51, 141));
        select_gyro_page2_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        quickWidget = new QQuickWidget(page_4);
        quickWidget->setObjectName("quickWidget");
        quickWidget->setGeometry(QRect(10, 10, 601, 301));
        quickWidget->setResizeMode(QQuickWidget::ResizeMode::SizeRootObjectToView);
        quickWidget->setSource(QUrl(QString::fromUtf8("")));
        stackedWidget->addWidget(page_4);
        page_5 = new QWidget();
        page_5->setObjectName("page_5");
        viewfinder = new QVideoWidget(page_5);
        viewfinder->setObjectName("viewfinder");
        viewfinder->setGeometry(QRect(-10, 0, 601, 311));
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(viewfinder->sizePolicy().hasHeightForWidth());
        viewfinder->setSizePolicy(sizePolicy1);
        pushButton_20 = new QPushButton(viewfinder);
        pushButton_20->setObjectName("pushButton_20");
        pushButton_20->setGeometry(QRect(270, 260, 151, 41));
        QFont font8;
        font8.setPointSize(19);
        font8.setBold(true);
        pushButton_20->setFont(font8);
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
        select_transponder_page2_3 = new QPushButton(page_5);
        select_transponder_page2_3->setObjectName("select_transponder_page2_3");
        select_transponder_page2_3->setGeometry(QRect(620, 10, 51, 141));
        select_transponder_page2_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
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
        select_gyro_page2_3->setGeometry(QRect(620, 170, 51, 141));
        select_gyro_page2_3->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
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
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        pushButton = new QPushButton(page_2);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(10, 10, 61, 61));
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
        pushButton_2->setGeometry(QRect(80, 10, 61, 61));
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
        pushButton_3->setGeometry(QRect(150, 10, 61, 61));
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
        pushButton_4->setGeometry(QRect(10, 80, 61, 61));
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
        pushButton_5->setGeometry(QRect(80, 80, 61, 61));
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
        pushButton_6->setGeometry(QRect(150, 80, 61, 61));
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
        pushButton_7->setGeometry(QRect(10, 150, 61, 61));
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
        pushButton_8->setGeometry(QRect(80, 150, 61, 61));
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
        pushButton_9->setGeometry(QRect(150, 150, 61, 61));
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
        pushButton_16->setGeometry(QRect(10, 220, 61, 61));
        QFont font9;
        font9.setBold(true);
        pushButton_16->setFont(font9);
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
        pushButton_17->setGeometry(QRect(80, 220, 61, 61));
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
        pushButton_18->setGeometry(QRect(150, 220, 61, 61));
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
        pushButton_14->setGeometry(QRect(10, 290, 61, 21));
        QFont font10;
        font10.setPointSize(10);
        pushButton_14->setFont(font10);
        pushButton_14->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(0, 0, 0);\n"
"border: 2px solid #555;\n"
"border-radius: 10px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #900\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
""));
        pushButton_11 = new QPushButton(page_2);
        pushButton_11->setObjectName("pushButton_11");
        pushButton_11->setGeometry(QRect(80, 290, 61, 21));
        pushButton_11->setFont(font10);
        pushButton_11->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(0, 0, 0);\n"
"border: 2px solid #555;\n"
"border-radius: 10px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #900\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
""));
        pushButton_11->setCheckable(false);
        tabWidget = new QTabWidget(page_2);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(220, 140, 231, 171));
        QFont font11;
        font11.setFamilies({QString::fromUtf8("Times New Roman")});
        font11.setPointSize(22);
        tabWidget->setFont(font11);
        tabWidget->setStyleSheet(QString::fromUtf8("color: rgb(155, 155, 155);\n"
"selection-color: rgb(253, 255, 135);\n"
"\n"
""));
        tab = new QWidget();
        tab->setObjectName("tab");
        label_2 = new QLabel(tab);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(170, 90, 61, 31));
        label_2->setFont(font);
        label_2->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        lcdNumber_3 = new QLCDNumber(tab);
        lcdNumber_3->setObjectName("lcdNumber_3");
        lcdNumber_3->setGeometry(QRect(0, 0, 231, 101));
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
        plainTextEdit->setGeometry(QRect(10, 0, 211, 121));
        QFont font12;
        font12.setPointSize(14);
        plainTextEdit->setFont(font12);
        plainTextEdit->setStyleSheet(QString::fromUtf8("color: rgb(238, 250, 255);"));
        plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
        plainTextEdit->setReadOnly(true);
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        pushButton_19 = new QPushButton(tab_3);
        pushButton_19->setObjectName("pushButton_19");
        pushButton_19->setGeometry(QRect(120, 80, 101, 51));
        QFont font13;
        font13.setPointSize(20);
        font13.setBold(true);
        pushButton_19->setFont(font13);
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
        pushButton_off->setGeometry(QRect(10, 80, 101, 51));
        QFont font14;
        font14.setPointSize(16);
        font14.setBold(true);
        pushButton_off->setFont(font14);
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
        pushButton_12->setGeometry(QRect(120, 0, 101, 71));
        pushButton_12->setFont(font13);
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
        pushButton_13->setGeometry(QRect(10, 0, 101, 71));
        pushButton_13->setFont(font13);
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
        tabWidget->addTab(tab_3, QString());
        lcdNumber = new QLCDNumber(page_2);
        lcdNumber->setObjectName("lcdNumber");
        lcdNumber->setGeometry(QRect(220, 0, 291, 151));
        lcdNumber->setFont(font9);
        lcdNumber->setStyleSheet(QString::fromUtf8("color: rgb(0, 223, 15);"));
        lcdNumber->setDigitCount(4);
        lcdNumber->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber->setProperty("intValue", QVariant(8888));
        pushButton_Ident = new QPushButton(page_2);
        pushButton_Ident->setObjectName("pushButton_Ident");
        pushButton_Ident->setGeometry(QRect(530, 10, 81, 41));
        QFont font15;
        font15.setPointSize(25);
        font15.setBold(true);
        pushButton_Ident->setFont(font15);
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
        pushButton_stby->setGeometry(QRect(530, 60, 81, 41));
        pushButton_stby->setFont(font15);
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
        pushButton_norm->setGeometry(QRect(530, 110, 81, 41));
        pushButton_norm->setFont(font15);
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
        pushButton_alt->setGeometry(QRect(530, 160, 81, 41));
        pushButton_alt->setFont(font15);
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
        lcdNumber_2->setGeometry(QRect(450, 220, 161, 81));
        QFont font16;
        font16.setItalic(true);
        lcdNumber_2->setFont(font16);
        lcdNumber_2->setStyleSheet(QString::fromUtf8("color: rgb(255, 243, 58);"));
        lcdNumber_2->setDigitCount(4);
        lcdNumber_2->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber_2->setProperty("intValue", QVariant(8888));
        pushButton_10 = new QPushButton(page_2);
        pushButton_10->setObjectName("pushButton_10");
        pushButton_10->setGeometry(QRect(150, 290, 61, 21));
        QFont font17;
        font17.setPointSize(11);
        pushButton_10->setFont(font17);
        pushButton_10->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(0, 0, 0);\n"
"border: 2px solid #555;\n"
"border-radius: 10px;\n"
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
        select_gyro_page->setGeometry(QRect(620, 10, 51, 141));
        select_gyro_page->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
        select_dumy_page = new QPushButton(page_2);
        select_dumy_page->setObjectName("select_dumy_page");
        select_dumy_page->setGeometry(QRect(620, 170, 51, 141));
        select_dumy_page->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 20px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #F60\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: rgb(224, 0, 0);\n"
"    border-style: inset;\n"
"}\n"
""));
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
        select_dumy_page->raise();
        MainWindow_small->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow_small);
        statusbar->setObjectName("statusbar");
        statusbar->setSizeGripEnabled(true);
        MainWindow_small->setStatusBar(statusbar);

        retranslateUi(MainWindow_small);

        stackedWidget->setCurrentIndex(0);
        imu_reset->setDefault(true);
        exit_2->setDefault(true);
        tabWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow_small);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow_small)
    {
        MainWindow_small->setWindowTitle(QCoreApplication::translate("MainWindow_small", "MainWindow", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow_small", "UTC Time:", nullptr));
        timeEdit->setDisplayFormat(QCoreApplication::translate("MainWindow_small", "MM/dd HH:mm", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow_small", "Flight timer:", nullptr));
        timeEdit_2->setDisplayFormat(QCoreApplication::translate("MainWindow_small", "HH:mm:ss", nullptr));
        compass->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        speed->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        roll->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        pitch->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        select_dumy_page2->setText(QCoreApplication::translate("MainWindow_small", "Next", nullptr));
        select_transponder_page->setText(QCoreApplication::translate("MainWindow_small", "Prev.", nullptr));
        timer_start->setText(QCoreApplication::translate("MainWindow_small", "ViewLogFile", nullptr));
        imu_reset->setText(QCoreApplication::translate("MainWindow_small", "Reset Pitch", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow_small", "Bearing", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow_small", "Roll", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow_small", "Pitch", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow_small", "Speed km/h", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow_small", "Alt. ft.", nullptr));
        altitude->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        temp->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow_small", "Temp.", nullptr));
        exit_2->setText(QCoreApplication::translate("MainWindow_small", "Exit", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow_small", "Artificial Horizon:", nullptr));
        label_15->setText(QCoreApplication::translate("MainWindow_small", "Temperature", nullptr));
        temperature->setText(QCoreApplication::translate("MainWindow_small", "0.0", nullptr));
        select_transponder_page2->setText(QCoreApplication::translate("MainWindow_small", "Next", nullptr));
        select_gyro_page2->setText(QCoreApplication::translate("MainWindow_small", "Prev.", nullptr));
        textEdit->setHtml(QCoreApplication::translate("MainWindow_small", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:18pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:13pt;\"><br /></p></body></html>", nullptr));
        textEdit_2->setHtml(QCoreApplication::translate("MainWindow_small", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'.AppleSystemUIFont'; font-size:18pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:13pt;\"><br /></p></body></html>", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow_small", "Radio frequencies:", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow_small", "Airplanes:", nullptr));
        pushButton_15->setText(QCoreApplication::translate("MainWindow_small", "Reset List", nullptr));
        select_transponder_page2_2->setText(QCoreApplication::translate("MainWindow_small", "Next", nullptr));
        select_gyro_page2_2->setText(QCoreApplication::translate("MainWindow_small", "Prev.", nullptr));
        pushButton_20->setText(QCoreApplication::translate("MainWindow_small", "Take Picture", nullptr));
        select_transponder_page2_3->setText(QCoreApplication::translate("MainWindow_small", "Next", nullptr));
        select_gyro_page2_3->setText(QCoreApplication::translate("MainWindow_small", "Prev.", nullptr));
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
        pushButton_14->setText(QCoreApplication::translate("MainWindow_small", "Com. OK", nullptr));
        pushButton_11->setText(QCoreApplication::translate("MainWindow_small", "Alt. OK", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow_small", "Alt. ft.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow_small", "Alt", nullptr));
        plainTextEdit->setPlainText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow_small", "Status", nullptr));
        pushButton_19->setText(QCoreApplication::translate("MainWindow_small", "Exit", nullptr));
        pushButton_off->setText(QCoreApplication::translate("MainWindow_small", "HW check", nullptr));
        pushButton_12->setText(QCoreApplication::translate("MainWindow_small", "Meter", nullptr));
        pushButton_13->setText(QCoreApplication::translate("MainWindow_small", "Feet", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("MainWindow_small", "Config", nullptr));
        pushButton_Ident->setText(QCoreApplication::translate("MainWindow_small", "IDENT", nullptr));
        pushButton_stby->setText(QCoreApplication::translate("MainWindow_small", "STBY", nullptr));
        pushButton_norm->setText(QCoreApplication::translate("MainWindow_small", "Norm", nullptr));
        pushButton_alt->setText(QCoreApplication::translate("MainWindow_small", "Alt", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow_small", "Ping OK", nullptr));
        select_gyro_page->setText(QCoreApplication::translate("MainWindow_small", "Next", nullptr));
        select_dumy_page->setText(QCoreApplication::translate("MainWindow_small", "Prev.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow_small: public Ui_MainWindow_small {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_SMALL_H
