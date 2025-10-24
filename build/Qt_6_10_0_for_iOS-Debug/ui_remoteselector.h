/********************************************************************************
** Form generated from reading UI file 'remoteselector.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REMOTESELECTOR_H
#define UI_REMOTESELECTOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_RemoteSelector
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *status;
    QListWidget *remoteDevices;
    QPushButton *connectButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *RemoteSelector)
    {
        if (RemoteSelector->objectName().isEmpty())
            RemoteSelector->setObjectName("RemoteSelector");
        RemoteSelector->resize(352, 315);
        verticalLayout = new QVBoxLayout(RemoteSelector);
        verticalLayout->setObjectName("verticalLayout");
        status = new QLabel(RemoteSelector);
        status->setObjectName("status");

        verticalLayout->addWidget(status);

        remoteDevices = new QListWidget(RemoteSelector);
        remoteDevices->setObjectName("remoteDevices");

        verticalLayout->addWidget(remoteDevices);

        connectButton = new QPushButton(RemoteSelector);
        connectButton->setObjectName("connectButton");
        connectButton->setEnabled(false);

        verticalLayout->addWidget(connectButton);

        cancelButton = new QPushButton(RemoteSelector);
        cancelButton->setObjectName("cancelButton");

        verticalLayout->addWidget(cancelButton);


        retranslateUi(RemoteSelector);

        QMetaObject::connectSlotsByName(RemoteSelector);
    } // setupUi

    void retranslateUi(QDialog *RemoteSelector)
    {
        RemoteSelector->setWindowTitle(QCoreApplication::translate("RemoteSelector", "Available chat services", nullptr));
        status->setText(QCoreApplication::translate("RemoteSelector", "Scanning...", nullptr));
        connectButton->setText(QCoreApplication::translate("RemoteSelector", "Connect", nullptr));
        cancelButton->setText(QCoreApplication::translate("RemoteSelector", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RemoteSelector: public Ui_RemoteSelector {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REMOTESELECTOR_H
