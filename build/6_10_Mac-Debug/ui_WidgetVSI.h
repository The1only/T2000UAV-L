/********************************************************************************
** Form generated from reading UI file 'WidgetVSI.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETVSI_H
#define UI_WIDGETVSI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "qfi/qfi_VSI.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetVSI
{
public:
    qfi_VSI *graphicsVSI;

    void setupUi(QWidget *WidgetVSI)
    {
        if (WidgetVSI->objectName().isEmpty())
            WidgetVSI->setObjectName("WidgetVSI");
        WidgetVSI->resize(400, 300);
        graphicsVSI = new qfi_VSI(WidgetVSI);
        graphicsVSI->setObjectName("graphicsVSI");
        graphicsVSI->setEnabled(false);
        graphicsVSI->setGeometry(QRect(0, 1, 401, 301));
        graphicsVSI->setFrameShape(QFrame::Shape::NoFrame);
        graphicsVSI->setFrameShadow(QFrame::Shadow::Plain);
        graphicsVSI->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsVSI->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsVSI->setInteractive(false);

        retranslateUi(WidgetVSI);

        QMetaObject::connectSlotsByName(WidgetVSI);
    } // setupUi

    void retranslateUi(QWidget *WidgetVSI)
    {
        WidgetVSI->setWindowTitle(QCoreApplication::translate("WidgetVSI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetVSI: public Ui_WidgetVSI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETVSI_H
