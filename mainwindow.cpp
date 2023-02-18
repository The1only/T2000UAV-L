#include <QtCore/QLoggingCategory>
#include <QQmlContext>
#include <QGuiApplication>
#include <QColorDialog>
#include <QNetworkInterface>
#include <QTimer>
#include <iostream>
#include <cstdio>
#include <QApplication>

#include "mainwindow.h"
#include "ui_mainwindow_port.h"
#include "ui_mainwindow_small.h"
#include "ui_mainwindow.h"
#include "mytcpsocket.h"

static MainWindow *saved_this;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    saved_this = this;

    QScreen *s = QGuiApplication::primaryScreen();
    if(s != NULL){
        qDebug() << "nativeOrientation: " << s->nativeOrientation();
        qDebug() << "orientation: " << s->orientation();
        qDebug() << (s->isLandscape(s->nativeOrientation()) ? "nativeOrientation lanscape" : "nativeOrientation not lanscape");
        qDebug() << (s->isLandscape(s->orientation()) ? "orientation lanscape" : "orientation not lanscape");
        qDebug() << (s->isPortrait(s->nativeOrientation()) ? "nativeOrientation portrait" : "nativeOrientation not portrait");
        qDebug() << (s->isPortrait(s->orientation()) ? "orientation portrait" : "orientation not portrait");
        ScreenMode = s->orientation();
        if (ScreenMode == Qt::InvertedPortraitOrientation) ScreenMode = Qt::PortraitOrientation;
        if(s->isPortrait(s->orientation()))
        {
            ui2 = new Ui::MainWindow_port;
            ui2->setupUi(this);
            ui2->lcdNumber->display(QString::number(this->current[0]*1000+this->current[1]*100+this->current[2]*10+this->current[3]).rightJustified(4, '0'));
            ui2->lcdNumber_2->display(QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0'));
            mysocket = new MyTcpSocket(this, ui2->plainTextEdit, &this->getVal);
        }
        else{
            ui = new Ui::MainWindow;
            ui->setupUi(this);
            ui->lcdNumber->display(QString::number(this->current[0]*1000+this->current[1]*100+this->current[2]*10+this->current[3]).rightJustified(4, '0'));
            ui->lcdNumber_2->display(QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0'));
            mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal);
        }
    }
    setmode(0);

    mysocket->doConnect();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getVal(QByteArray array)
{
    static char buffer[30];
    static int pos = 0;
 //   qDebug() << "Reseive: " << array;

    for(int i=0; i < array.size();i++){
        if(array.at(i) < 0x1F || pos >= sizeof(buffer)){
      //    qDebug() << "R: >>buffer;  //m_message);

            if(pos >= 3){
                switch (buffer[0]){
                case 's':
                {
                    QString x;
                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        x = saved_this->ui2->pushButton_off->styleSheet();
                        x.replace(QString("1 #561"), QString("1 #888"));
                        saved_this->ui2->pushButton_off->setStyleSheet(x);
                        saved_this->ui2->pushButton_norm->setStyleSheet(x);
                        saved_this->ui2->pushButton_stby->setStyleSheet(x);
                        saved_this->ui2->pushButton_alt->setStyleSheet(x);
                    }
                    else{
                        x = saved_this->ui->pushButton_off->styleSheet();
                        x.replace(QString("1 #561"), QString("1 #888"));
                        saved_this->ui->pushButton_off->setStyleSheet(x);
                        saved_this->ui->pushButton_norm->setStyleSheet(x);
                        saved_this->ui->pushButton_stby->setStyleSheet(x);
                        saved_this->ui->pushButton_alt->setStyleSheet(x);
                    }
                    x.replace(QString("1 #888"), QString("1 #561"));

                    switch(buffer[2]){
                    case 'o':
                        if(saved_this->ScreenMode == Qt::PortraitOrientation){
                            saved_this->ui2->pushButton_off->setStyleSheet(x);
                        }
                        else{
                            saved_this->ui->pushButton_off->setStyleSheet(x);
                        }
                        saved_this->mode = 0;
                        break;
                    case 't':
                        if(saved_this->ScreenMode == Qt::PortraitOrientation){
                            saved_this->ui2->pushButton_stby->setStyleSheet(x);
                        }
                        else{
                            saved_this->ui->pushButton_stby->setStyleSheet(x);
                        }
                        saved_this->mode = 1;
                        break;
                    case 'a':
                        if(saved_this->ScreenMode == Qt::PortraitOrientation){
                            saved_this->ui2->pushButton_norm->setStyleSheet(x);
                        }
                        else{
                            saved_this->ui->pushButton_norm->setStyleSheet(x);
                        }
                        saved_this->mode = 2;
                        break;
                    case 'c':
                        if(saved_this->ScreenMode == Qt::PortraitOrientation){
                            saved_this->ui2->pushButton_alt->setStyleSheet(x);
                        }
                        else{
                            saved_this->ui->pushButton_alt->setStyleSheet(x);
                        }
                        saved_this->mode = 3;
                        break;
                    }
                    break;
                }
                case 'r':
                {
                    bool state=true;
                    if (buffer[2] == 'N') state = false;

                    QString x;
                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        x = saved_this->ui2->pushButton_10->styleSheet();
                    }
                    else{
                        x = saved_this->ui->pushButton_10->styleSheet();
                    }

                    if ( state == false){
                        x.replace(QString("1 #090"), QString("1 #900"));
                    }else{
                        x.replace(QString("1 #900"), QString("1 #090"));
                    }

                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        saved_this->ui->pushButton_10->setStyleSheet(x);
                        saved_this->ui->pushButton_10->update();
                    }
                    else{
                        saved_this->ui->pushButton_10->setStyleSheet(x);
                        saved_this->ui->pushButton_10->update();
                    }
                    break;
                }
                case 'i':
                {
                    bool state;
                    if (buffer[2] == '1') state = false;
                    else state = true;

                    QString x;
                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        x = saved_this->ui2->pushButton_Ident->styleSheet();
                    }
                    else{
                        x = saved_this->ui->pushButton_Ident->styleSheet();
                    }
                    if ( state == false){
                        x.replace(QString("1 #561"), QString("1 #953"));
                    }else{
                        x.replace(QString("1 #953"), QString("1 #561"));
                    }

                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        saved_this->ui->pushButton_Ident->setStyleSheet(x);
                        saved_this->ui->pushButton_Ident->update();
                    }
                    else{
                        saved_this->ui->pushButton_Ident->setStyleSheet(x);
                        saved_this->ui->pushButton_Ident->update();
                    }
                    break;
                }

                case 'c':
                {
                    int number;
                    char numout[5];
                    sscanf(buffer,"c=%d",&number);
                    sprintf(numout,"%.4d",number);

                    saved_this->current[3]=numout[3]-0x30;
                    saved_this->current[2]=numout[2]-0x30;
                    saved_this->current[1]=numout[1]-0x30;
                    saved_this->current[0]=numout[0]-0x30;
                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        saved_this->ui2->lcdNumber->display(QString::number(saved_this->current[0]*1000+
                                                            saved_this->current[1]*100+
                                saved_this->current[2]*10+
                                saved_this->current[3]).rightJustified(4, '0'));
                    }
                    else{
                        saved_this->ui->lcdNumber->display(QString::number(saved_this->current[0]*1000+
                                                           saved_this->current[1]*100+
                                saved_this->current[2]*10+
                                saved_this->current[3]).rightJustified(4, '0'));
                    }
                    break;
                }
                case 'a':
                {
                    float number;
                    char numout[10];
                    sscanf(buffer,"a=%f",&number);

                    for (unsigned long key=0; key < strlen(buffer); key++) if(buffer[key]=='M') {number*=3.2808399; break;}
                    sprintf(numout,"%.4d",(int)number);

                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        saved_this->ui2->lcdNumber_3->display(numout);
                    }
                    else{
                        saved_this->ui->lcdNumber_3->display(numout);
                    }
                    break;
                }
                case 'z':
                {
                    qDebug() << "T: %s\r\n" << &buffer[2];

                    if(saved_this->ScreenMode == Qt::PortraitOrientation){
                        saved_this->ui2->plainTextEdit->appendPlainText(&buffer[2]);
                    }
                    else{
                        saved_this->ui->plainTextEdit->appendPlainText(&buffer[2]);
                    }
                    break;
                }
                }
            }
            pos = 0;

        }
        else{
            if(array.at(i) >= 0x1F){
                buffer[pos++]=array.at(i);
                buffer[pos]=0;
                //               std::fprintf(stderr,"T: %s\r\n",buffer);  //m_message);
            }
        }
    }


}

void MainWindow::setVal()
{
    /*
    QByteArray array = device.m_message.toLocal8Bit();
    char* buffer = array.data();
    std::fprintf(stderr,"M: %s \n\r",buffer);  //m_message);

    ui->plainTextEdit->appendPlainText(device.m_message);
    */
}


void MainWindow::accepted(void)
{
    this->close();
}

void MainWindow::addnext(int x)
{
    this->next[0]=this->next[1];
    this->next[1]=this->next[2];
    this->next[2]=this->next[3];
    this->next[3]=x;

    QString num = QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0');
    if(saved_this->ScreenMode == Qt::PortraitOrientation){
        ui2->lcdNumber_2->display( num);
    }
    else{
        ui->lcdNumber_2->display( num);
    }
}

void MainWindow::setmode(int m)
{
    qDebug() << "Mode: " << m;
    //    if( mode != m & mysocket != NULL)
    if(mysocket != NULL)
    {
        switch(m){
        case 0: mysocket->readyWrite("s=?\r\n"); break;
        case 1: mysocket->readyWrite("s=t\r\n"); break;
        case 2: mysocket->readyWrite("s=a\r\n"); break;
        case 3: mysocket->readyWrite("s=c\r\n"); break;
        }
    }
    mode = m;
}

void MainWindow::on_pushButton_clicked(  ){addnext(1);}
void MainWindow::on_pushButton_2_clicked(){addnext(2);}
void MainWindow::on_pushButton_3_clicked(){addnext(3);}
void MainWindow::on_pushButton_4_clicked(){addnext(4);}
void MainWindow::on_pushButton_5_clicked(){addnext(5);}
void MainWindow::on_pushButton_6_clicked(){addnext(6);}
void MainWindow::on_pushButton_7_clicked(){addnext(7);}
void MainWindow::on_pushButton_8_clicked(){addnext(8);}
void MainWindow::on_pushButton_9_clicked(){addnext(9);}
void MainWindow::on_pushButton_17_clicked(){addnext(0);}

void MainWindow::on_pushButton_19_clicked()
{
    this->close();
}

void MainWindow::on_pushButton_16_clicked()
{
    this->current[3]=this->next[3];
    this->current[2]=this->next[2];
    this->current[1]=this->next[1];
    this->current[0]=this->next[0];

    char data[100];
    sprintf(data,"c=%d\r\n",this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]);
//    QString num = QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]);
//    QString msg = QString("c=%1\r\n").arg(num);
    qDebug() << data;
    mysocket->readyWrite(data);

}

void MainWindow::on_pushButton_18_clicked()
{
    //   device.startDeviceDiscovery();
    //    view.show();

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost){
            qDebug() << address.toString();
            if(saved_this->ScreenMode == Qt::PortraitOrientation){
                ui2->plainTextEdit->appendPlainText(address.toString());
            }
            else{
                ui->plainTextEdit->appendPlainText(address.toString());
            }
        }
    }
}


void MainWindow::on_pushButton_Ident_clicked()
{
    static bool state = false;

    if ( state == false){
        state = true;
        mysocket->readyWrite("i=1\r\n");
    }else{
        state = false;
        mysocket->readyWrite("i=0\r\n");
    }
}

// Set mode...
void MainWindow::on_pushButton_stby_clicked(){setmode(1);}
void MainWindow::on_pushButton_off_clicked(){setmode(0);}
void MainWindow::on_pushButton_norm_clicked(){setmode(2);}
void MainWindow::on_pushButton_alt_clicked(){setmode(3);}

