/********************************************************************************
** Form generated from reading UI file 'WidgetHI.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETHI_H
#define UI_WIDGETHI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "qfi/qfi_HI.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetHI
{
public:
    qfi_HI *graphicsHI;

    void setupUi(QWidget *WidgetHI)
    {
        if (WidgetHI->objectName().isEmpty())
            WidgetHI->setObjectName("WidgetHI");
        WidgetHI->resize(400, 300);
        graphicsHI = new qfi_HI(WidgetHI);
        graphicsHI->setObjectName("graphicsHI");
        graphicsHI->setEnabled(false);
        graphicsHI->setGeometry(QRect(0, 1, 401, 291));
        graphicsHI->setFrameShape(QFrame::Shape::NoFrame);
        graphicsHI->setFrameShadow(QFrame::Shadow::Plain);
        graphicsHI->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsHI->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsHI->setInteractive(false);

        retranslateUi(WidgetHI);

        QMetaObject::connectSlotsByName(WidgetHI);
    } // setupUi

    void retranslateUi(QWidget *WidgetHI)
    {
        WidgetHI->setWindowTitle(QCoreApplication::translate("WidgetHI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetHI: public Ui_WidgetHI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETHI_H
