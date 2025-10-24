/********************************************************************************
** Form generated from reading UI file 'WidgetEADI.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGETEADI_H
#define UI_WIDGETEADI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>
#include <qfi/qfi_EADI.h>

QT_BEGIN_NAMESPACE

class Ui_WidgetEADI
{
public:
    QFrame *frameEADI;
    QGridLayout *gridLayout;
    qfi_EADI *graphicsEADI;

    void setupUi(QWidget *WidgetEADI)
    {
        if (WidgetEADI->objectName().isEmpty())
            WidgetEADI->setObjectName("WidgetEADI");
        WidgetEADI->resize(400, 400);
        frameEADI = new QFrame(WidgetEADI);
        frameEADI->setObjectName("frameEADI");
        frameEADI->setGeometry(QRect(70, 70, 260, 260));
        frameEADI->setStyleSheet(QString::fromUtf8("#frameEADI\n"
"{\n"
"	background-color: #000;\n"
"	border-radius: 10px;\n"
"}"));
        frameEADI->setFrameShape(QFrame::Shape::StyledPanel);
        frameEADI->setFrameShadow(QFrame::Shadow::Raised);
        gridLayout = new QGridLayout(frameEADI);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(4, 4, 4, 4);
        graphicsEADI = new qfi_EADI(frameEADI);
        graphicsEADI->setObjectName("graphicsEADI");
        graphicsEADI->setEnabled(false);
        graphicsEADI->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        graphicsEADI->setFrameShape(QFrame::Shape::NoFrame);
        graphicsEADI->setFrameShadow(QFrame::Shadow::Plain);
        graphicsEADI->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsEADI->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        graphicsEADI->setInteractive(false);
        graphicsEADI->setOptimizationFlags(QGraphicsView::OptimizationFlag::IndirectPainting);

        gridLayout->addWidget(graphicsEADI, 0, 0, 1, 1);


        retranslateUi(WidgetEADI);

        QMetaObject::connectSlotsByName(WidgetEADI);
    } // setupUi

    void retranslateUi(QWidget *WidgetEADI)
    {
        WidgetEADI->setWindowTitle(QCoreApplication::translate("WidgetEADI", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WidgetEADI: public Ui_WidgetEADI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGETEADI_H
