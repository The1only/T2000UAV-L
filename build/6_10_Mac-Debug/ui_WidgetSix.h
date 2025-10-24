/********************************************************************************
** Form generated from reading UI file 'WidgetSix.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETSIX_H
#define UI_WIDGETSIX_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include "example/WidgetAI.h"
#include "example/WidgetALT.h"
#include "example/WidgetASI.h"
#include "example/WidgetHI.h"
#include "example/WidgetTC.h"
#include "example/WidgetVSI.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetSix
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    WidgetASI *widgetASI;
    WidgetAI *widgetAI;
    WidgetALT *widgetALT;
    WidgetTC *widgetTC;
    WidgetHI *widgetHI;
    WidgetVSI *widgetVSI;

    void setupUi(QWidget *WidgetSix)
    {
        if (WidgetSix->objectName().isEmpty())
            WidgetSix->setObjectName("WidgetSix");
        WidgetSix->resize(400, 300);
        gridLayout_2 = new QGridLayout(WidgetSix);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        widgetASI = new WidgetASI(WidgetSix);
        widgetASI->setObjectName("widgetASI");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(99);
        sizePolicy.setVerticalStretch(99);
        sizePolicy.setHeightForWidth(widgetASI->sizePolicy().hasHeightForWidth());
        widgetASI->setSizePolicy(sizePolicy);

        gridLayout->addWidget(widgetASI, 0, 0, 1, 1);

        widgetAI = new WidgetAI(WidgetSix);
        widgetAI->setObjectName("widgetAI");
        sizePolicy.setHeightForWidth(widgetAI->sizePolicy().hasHeightForWidth());
        widgetAI->setSizePolicy(sizePolicy);

        gridLayout->addWidget(widgetAI, 0, 1, 1, 1);

        widgetALT = new WidgetALT(WidgetSix);
        widgetALT->setObjectName("widgetALT");
        sizePolicy.setHeightForWidth(widgetALT->sizePolicy().hasHeightForWidth());
        widgetALT->setSizePolicy(sizePolicy);

        gridLayout->addWidget(widgetALT, 0, 2, 1, 1);

        widgetTC = new WidgetTC(WidgetSix);
        widgetTC->setObjectName("widgetTC");
        sizePolicy.setHeightForWidth(widgetTC->sizePolicy().hasHeightForWidth());
        widgetTC->setSizePolicy(sizePolicy);

        gridLayout->addWidget(widgetTC, 1, 0, 1, 1);

        widgetHI = new WidgetHI(WidgetSix);
        widgetHI->setObjectName("widgetHI");
        sizePolicy.setHeightForWidth(widgetHI->sizePolicy().hasHeightForWidth());
        widgetHI->setSizePolicy(sizePolicy);

        gridLayout->addWidget(widgetHI, 1, 1, 1, 1);

        widgetVSI = new WidgetVSI(WidgetSix);
        widgetVSI->setObjectName("widgetVSI");
        sizePolicy.setHeightForWidth(widgetVSI->sizePolicy().hasHeightForWidth());
        widgetVSI->setSizePolicy(sizePolicy);

        gridLayout->addWidget(widgetVSI, 1, 2, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);


        retranslateUi(WidgetSix);

        QMetaObject::connectSlotsByName(WidgetSix);
    } // setupUi

    void retranslateUi(QWidget *WidgetSix)
    {
        WidgetSix->setWindowTitle(QCoreApplication::translate("WidgetSix", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetSix: public Ui_WidgetSix {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETSIX_H
