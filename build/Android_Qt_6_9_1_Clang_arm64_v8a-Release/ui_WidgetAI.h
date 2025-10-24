/********************************************************************************
** Form generated from reading UI file 'WidgetAI.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETAI_H
#define UI_WIDGETAI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include "qfi/qfi_AI.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetAI
{
public:
    qfi_AI *graphicsAI;

    void setupUi(QWidget *WidgetAI)
    {
        if (WidgetAI->objectName().isEmpty())
            WidgetAI->setObjectName("WidgetAI");
        WidgetAI->resize(400, 300);
        graphicsAI = new qfi_AI(WidgetAI);
        graphicsAI->setObjectName("graphicsAI");
        graphicsAI->setEnabled(false);
        graphicsAI->setGeometry(QRect(5, 0, 391, 301));
        graphicsAI->setFrameShape(QFrame::Shape::NoFrame);
        graphicsAI->setFrameShadow(QFrame::Shadow::Plain);
        graphicsAI->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsAI->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsAI->setInteractive(false);

        retranslateUi(WidgetAI);

        QMetaObject::connectSlotsByName(WidgetAI);
    } // setupUi

    void retranslateUi(QWidget *WidgetAI)
    {
        WidgetAI->setWindowTitle(QCoreApplication::translate("WidgetAI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetAI: public Ui_WidgetAI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETAI_H
