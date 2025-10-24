/********************************************************************************
** Form generated from reading UI file 'mainwindow_phone.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_PHONE_H
#define UI_MAINWINDOW_PHONE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;
    QPushButton *pushButton_off;
    QPushButton *pushButton_stby;
    QPushButton *pushButton_norm;
    QPushButton *pushButton_alt;
    QLCDNumber *lcdNumber;
    QPushButton *pushButton_16;
    QPushButton *pushButton_17;
    QPushButton *pushButton_18;
    QPushButton *pushButton_19;
    QPushButton *pushButton_Ident;
    QTabWidget *tabWidget;
    QWidget *tab;
    QLabel *label_2;
    QLCDNumber *lcdNumber_3;
    QWidget *tab_4;
    QLCDNumber *lcdNumber_2;
    QLabel *label;
    QWidget *tab_2;
    QPlainTextEdit *plainTextEdit;
    QWidget *tab_3;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(459, 768);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(10, 768));
        MainWindow->setMaximumSize(QSize(1024, 768));
        MainWindow->setStyleSheet(QString::fromUtf8("background-color: rgb(161, 161, 161);"));
        MainWindow->setUnifiedTitleAndToolBarOnMac(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(20, 360, 101, 91));
        pushButton->setStyleSheet(QString::fromUtf8("image: url(:/one.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(130, 360, 101, 91));
        pushButton_2->setStyleSheet(QString::fromUtf8("image: url(:/two.png);\n"
"background-image: url(:/two.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_3 = new QPushButton(centralwidget);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(240, 360, 101, 91));
        pushButton_3->setStyleSheet(QString::fromUtf8("image: url(:/tree.png);\n"
"background-image: url(:/two.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_4 = new QPushButton(centralwidget);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(20, 460, 101, 91));
        pushButton_4->setStyleSheet(QString::fromUtf8("image: url(:/fore.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_5 = new QPushButton(centralwidget);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(130, 460, 101, 91));
        pushButton_5->setStyleSheet(QString::fromUtf8("image: url(:/five.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_6 = new QPushButton(centralwidget);
        pushButton_6->setObjectName("pushButton_6");
        pushButton_6->setGeometry(QRect(240, 460, 101, 91));
        pushButton_6->setStyleSheet(QString::fromUtf8("image: url(:/six.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_7 = new QPushButton(centralwidget);
        pushButton_7->setObjectName("pushButton_7");
        pushButton_7->setGeometry(QRect(20, 560, 101, 81));
        pushButton_7->setStyleSheet(QString::fromUtf8("image: url(:/seven.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_8 = new QPushButton(centralwidget);
        pushButton_8->setObjectName("pushButton_8");
        pushButton_8->setGeometry(QRect(130, 560, 101, 81));
        pushButton_8->setStyleSheet(QString::fromUtf8("image: url(:/eight.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_9 = new QPushButton(centralwidget);
        pushButton_9->setObjectName("pushButton_9");
        pushButton_9->setGeometry(QRect(240, 560, 101, 81));
        pushButton_9->setStyleSheet(QString::fromUtf8("image: url(:/nine.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_off = new QPushButton(centralwidget);
        pushButton_off->setObjectName("pushButton_off");
        pushButton_off->setGeometry(QRect(360, 200, 81, 81));
        QFont font;
        font.setPointSize(20);
        font.setBold(true);
        pushButton_off->setFont(font);
        pushButton_off->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(255, 166, 20);\n"
"border: 2px solid #555;\n"
"border-radius: 38px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.3,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #561\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
""));
        pushButton_stby = new QPushButton(centralwidget);
        pushButton_stby->setObjectName("pushButton_stby");
        pushButton_stby->setGeometry(QRect(360, 290, 81, 81));
        pushButton_stby->setFont(font);
        pushButton_stby->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 38px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"\n"
""));
        pushButton_norm = new QPushButton(centralwidget);
        pushButton_norm->setObjectName("pushButton_norm");
        pushButton_norm->setGeometry(QRect(360, 380, 81, 81));
        pushButton_norm->setFont(font);
        pushButton_norm->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 38px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
""));
        pushButton_alt = new QPushButton(centralwidget);
        pushButton_alt->setObjectName("pushButton_alt");
        pushButton_alt->setGeometry(QRect(360, 470, 81, 81));
        pushButton_alt->setFont(font);
        pushButton_alt->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"color: #333;\n"
"border: 2px solid #555;\n"
"border-radius: 38px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #888\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"\n"
""));
        lcdNumber = new QLCDNumber(centralwidget);
        lcdNumber->setObjectName("lcdNumber");
        lcdNumber->setGeometry(QRect(10, 200, 341, 151));
        QFont font1;
        font1.setBold(true);
        lcdNumber->setFont(font1);
        lcdNumber->setStyleSheet(QString::fromUtf8("color: rgb(0, 223, 15);"));
        lcdNumber->setDigitCount(4);
        lcdNumber->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber->setProperty("intValue", QVariant(8888));
        pushButton_16 = new QPushButton(centralwidget);
        pushButton_16->setObjectName("pushButton_16");
        pushButton_16->setGeometry(QRect(20, 650, 101, 91));
        pushButton_16->setStyleSheet(QString::fromUtf8("image: url(:/enter.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_17 = new QPushButton(centralwidget);
        pushButton_17->setObjectName("pushButton_17");
        pushButton_17->setGeometry(QRect(130, 650, 101, 91));
        pushButton_17->setStyleSheet(QString::fromUtf8("image: url(:/zero.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_18 = new QPushButton(centralwidget);
        pushButton_18->setObjectName("pushButton_18");
        pushButton_18->setGeometry(QRect(240, 650, 101, 91));
        pushButton_18->setStyleSheet(QString::fromUtf8("image: url(:/del.png);\n"
"background-image: url(:/one.png);\n"
"background-color: rgb(49, 49, 49);\n"
""));
        pushButton_19 = new QPushButton(centralwidget);
        pushButton_19->setObjectName("pushButton_19");
        pushButton_19->setGeometry(QRect(360, 650, 81, 81));
        pushButton_19->setFont(font);
        pushButton_19->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(255, 166, 20);\n"
"border: 2px solid #555;\n"
"border-radius: 38px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.4,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #557\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
""));
        pushButton_Ident = new QPushButton(centralwidget);
        pushButton_Ident->setObjectName("pushButton_Ident");
        pushButton_Ident->setGeometry(QRect(360, 560, 81, 81));
        pushButton_Ident->setFont(font);
        pushButton_Ident->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"background-color: rgb(255, 166, 20);\n"
"border: 2px solid #555;\n"
"border-radius: 38px;\n"
"border-style: outset;\n"
"background: qradialgradient(\n"
"cx: 0.5, cy: -0.4, fx: 0.3, fy: -0.3,\n"
"radius: 1.35, stop: 0 #fff, stop: 1 #561\n"
");\n"
"padding: 5px;\n"
"}\n"
"\n"
"\n"
""));
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(10, 10, 441, 181));
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Times New Roman")});
        font2.setPointSize(22);
        tabWidget->setFont(font2);
        tab = new QWidget();
        tab->setObjectName("tab");
        label_2 = new QLabel(tab);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(310, 80, 121, 41));
        QFont font3;
        font3.setPointSize(40);
        label_2->setFont(font3);
        lcdNumber_3 = new QLCDNumber(tab);
        lcdNumber_3->setObjectName("lcdNumber_3");
        lcdNumber_3->setGeometry(QRect(20, 0, 281, 141));
        lcdNumber_3->setStyleSheet(QString::fromUtf8("color: rgb(13, 255, 252);"));
        lcdNumber_3->setSmallDecimalPoint(false);
        lcdNumber_3->setDigitCount(5);
        lcdNumber_3->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber_3->setProperty("intValue", QVariant(88888));
        tabWidget->addTab(tab, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName("tab_4");
        lcdNumber_2 = new QLCDNumber(tab_4);
        lcdNumber_2->setObjectName("lcdNumber_2");
        lcdNumber_2->setGeometry(QRect(10, 10, 271, 131));
        QFont font4;
        font4.setItalic(true);
        lcdNumber_2->setFont(font4);
        lcdNumber_2->setStyleSheet(QString::fromUtf8("color: rgb(255, 243, 58);"));
        lcdNumber_2->setDigitCount(4);
        lcdNumber_2->setSegmentStyle(QLCDNumber::SegmentStyle::Flat);
        lcdNumber_2->setProperty("intValue", QVariant(8888));
        label = new QLabel(tab_4);
        label->setObjectName("label");
        label->setGeometry(QRect(290, 90, 81, 51));
        label->setFont(font3);
        tabWidget->addTab(tab_4, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        plainTextEdit = new QPlainTextEdit(tab_2);
        plainTextEdit->setObjectName("plainTextEdit");
        plainTextEdit->setGeometry(QRect(20, 0, 411, 141));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        tabWidget->addTab(tab_3, QString());
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        statusbar->setSizeGripEnabled(true);
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        pushButton->setText(QString());
        pushButton_2->setText(QString());
        pushButton_3->setText(QString());
        pushButton_4->setText(QString());
        pushButton_5->setText(QString());
        pushButton_6->setText(QString());
        pushButton_7->setText(QString());
        pushButton_8->setText(QString());
        pushButton_9->setText(QString());
        pushButton_off->setText(QCoreApplication::translate("MainWindow", "OFF", nullptr));
        pushButton_stby->setText(QCoreApplication::translate("MainWindow", "STBY", nullptr));
        pushButton_norm->setText(QCoreApplication::translate("MainWindow", "Norm", nullptr));
        pushButton_alt->setText(QCoreApplication::translate("MainWindow", "Alt", nullptr));
        pushButton_16->setText(QString());
        pushButton_17->setText(QString());
        pushButton_18->setText(QString());
        pushButton_19->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        pushButton_Ident->setText(QCoreApplication::translate("MainWindow", "IDENT", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Alt. ft.", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow", "Alt", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Next", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QCoreApplication::translate("MainWindow", "Squawk", nullptr));
        plainTextEdit->setPlainText(QCoreApplication::translate("MainWindow", "Status display...", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("MainWindow", "Status", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QCoreApplication::translate("MainWindow", "Config", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_PHONE_H
