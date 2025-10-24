/********************************************************************************
** Form generated from reading UI file 'WidgetALT.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETALT_H
#define UI_WIDGETALT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "qfi/qfi_ALT.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetALT
{
public:
    qfi_ALT *graphicsALT;

    void setupUi(QWidget *WidgetALT)
    {
        if (WidgetALT->objectName().isEmpty())
            WidgetALT->setObjectName("WidgetALT");
        WidgetALT->resize(400, 300);
        graphicsALT = new qfi_ALT(WidgetALT);
        graphicsALT->setObjectName("graphicsALT");
        graphicsALT->setEnabled(false);
        graphicsALT->setGeometry(QRect(0, 1, 401, 301));
        graphicsALT->setFrameShape(QFrame::Shape::NoFrame);
        graphicsALT->setFrameShadow(QFrame::Shadow::Plain);
        graphicsALT->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsALT->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsALT->setInteractive(false);

        retranslateUi(WidgetALT);

        QMetaObject::connectSlotsByName(WidgetALT);
    } // setupUi

    void retranslateUi(QWidget *WidgetALT)
    {
        WidgetALT->setWindowTitle(QCoreApplication::translate("WidgetALT", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetALT: public Ui_WidgetALT {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETALT_H
