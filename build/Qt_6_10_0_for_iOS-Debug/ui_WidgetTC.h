/********************************************************************************
** Form generated from reading UI file 'WidgetTC.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETTC_H
#define UI_WIDGETTC_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "qfi/qfi_TC.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetTC
{
public:
    qfi_TC *graphicsTC;

    void setupUi(QWidget *WidgetTC)
    {
        if (WidgetTC->objectName().isEmpty())
            WidgetTC->setObjectName("WidgetTC");
        WidgetTC->resize(400, 300);
        graphicsTC = new qfi_TC(WidgetTC);
        graphicsTC->setObjectName("graphicsTC");
        graphicsTC->setEnabled(false);
        graphicsTC->setGeometry(QRect(10, 11, 391, 281));
        graphicsTC->setFrameShape(QFrame::Shape::NoFrame);
        graphicsTC->setFrameShadow(QFrame::Shadow::Plain);
        graphicsTC->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsTC->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsTC->setInteractive(false);

        retranslateUi(WidgetTC);

        QMetaObject::connectSlotsByName(WidgetTC);
    } // setupUi

    void retranslateUi(QWidget *WidgetTC)
    {
        WidgetTC->setWindowTitle(QCoreApplication::translate("WidgetTC", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetTC: public Ui_WidgetTC {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETTC_H
