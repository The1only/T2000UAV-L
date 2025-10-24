/********************************************************************************
** Form generated from reading UI file 'WidgetASI.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETASI_H
#define UI_WIDGETASI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "qfi/qfi_ASI.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetASI
{
public:
    qfi_ASI *graphicsASI;

    void setupUi(QWidget *WidgetASI)
    {
        if (WidgetASI->objectName().isEmpty())
            WidgetASI->setObjectName("WidgetASI");
        WidgetASI->resize(400, 300);
        graphicsASI = new qfi_ASI(WidgetASI);
        graphicsASI->setObjectName("graphicsASI");
        graphicsASI->setEnabled(false);
        graphicsASI->setGeometry(QRect(0, 1, 401, 301));
        graphicsASI->setFrameShape(QFrame::Shape::NoFrame);
        graphicsASI->setFrameShadow(QFrame::Shadow::Plain);
        graphicsASI->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsASI->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsASI->setInteractive(false);

        retranslateUi(WidgetASI);

        QMetaObject::connectSlotsByName(WidgetASI);
    } // setupUi

    void retranslateUi(QWidget *WidgetASI)
    {
        WidgetASI->setWindowTitle(QCoreApplication::translate("WidgetASI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetASI: public Ui_WidgetASI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETASI_H
