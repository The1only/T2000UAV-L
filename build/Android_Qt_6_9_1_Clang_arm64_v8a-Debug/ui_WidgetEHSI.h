/********************************************************************************
** Form generated from reading UI file 'WidgetEHSI.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETEHSI_H
#define UI_WIDGETEHSI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include "qfi/qfi_EHSI.h"

QT_BEGIN_NAMESPACE

class Ui_WidgetEHSI
{
public:
    QFrame *frameEHSI;
    QGridLayout *gridLayout;
    qfi_EHSI *graphicsEHSI;

    void setupUi(QWidget *WidgetEHSI)
    {
        if (WidgetEHSI->objectName().isEmpty())
            WidgetEHSI->setObjectName("WidgetEHSI");
        WidgetEHSI->resize(400, 400);
        frameEHSI = new QFrame(WidgetEHSI);
        frameEHSI->setObjectName("frameEHSI");
        frameEHSI->setGeometry(QRect(70, 70, 260, 260));
        frameEHSI->setStyleSheet(QString::fromUtf8("#frameEHSI\n"
"{\n"
"	background-color: #000;\n"
"	border-radius: 10px;\n"
"}"));
        frameEHSI->setFrameShape(QFrame::Shape::StyledPanel);
        frameEHSI->setFrameShadow(QFrame::Shadow::Raised);
        gridLayout = new QGridLayout(frameEHSI);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(4, 4, 4, 4);
        graphicsEHSI = new qfi_EHSI(frameEHSI);
        graphicsEHSI->setObjectName("graphicsEHSI");
        graphicsEHSI->setEnabled(false);
        graphicsEHSI->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        graphicsEHSI->setFrameShape(QFrame::Shape::NoFrame);
        graphicsEHSI->setFrameShadow(QFrame::Shadow::Plain);
        graphicsEHSI->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsEHSI->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsEHSI->setInteractive(false);
        graphicsEHSI->setOptimizationFlags(QGraphicsView::OptimizationFlag::IndirectPainting);

        gridLayout->addWidget(graphicsEHSI, 0, 0, 1, 1);


        retranslateUi(WidgetEHSI);

        QMetaObject::connectSlotsByName(WidgetEHSI);
    } // setupUi

    void retranslateUi(QWidget *WidgetEHSI)
    {
        WidgetEHSI->setWindowTitle(QCoreApplication::translate("WidgetEHSI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetEHSI: public Ui_WidgetEHSI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETEHSI_H
