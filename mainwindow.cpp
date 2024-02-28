#include <QtCore/QLoggingCategory>
#include <QQmlContext>
#include <QGuiApplication>
#include <QColorDialog>
#include <QNetworkInterface>
#include <QTimer>
//#include <iostream>
#include <cstdio>
#include <QApplication>

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

#include "mainwindow.h"
#include "ui_mainwindow_port.h"
#include "ui_mainwindow_port_small.h"
#include "ui_mainwindow_small.h"
#include "ui_mainwindow.h"
#include "mytcpsocket.h"

static MainWindow *saved_this;

template<class T>
T clone(const T& orig) {
    return T{orig};
}

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
        QSizeF x = s->physicalSize();
        float Ssize = sqrt(( x.rheight() * x.rheight() ) + ( x.rwidth()* x.rwidth())) / 25.4;
        qDebug() << "Screen Size: " << x.rheight() << x.rwidth() << Ssize;;

        if (ScreenMode == Qt::InvertedPortraitOrientation) ScreenMode = Qt::PortraitOrientation;

        getauto();
        // int idx = xxz2->index();

        if (const auto intPtr (std::get_if<Ui::MainWindow_port_small>(xxz2)); intPtr) {ui = (Ui::MainWindow_small *)intPtr;}
        else if  (const auto intPtr (std::get_if<Ui::MainWindow_port>(xxz2)); intPtr) {ui = (Ui::MainWindow_small *)intPtr;}
        else if  (const auto intPtr (std::get_if<Ui::MainWindow_small>(xxz2)); intPtr) {ui = (Ui::MainWindow_small *)intPtr;}
        else if  (const auto intPtr (std::get_if<Ui::MainWindow>(xxz2)); intPtr) {ui = (Ui::MainWindow_small *)intPtr;}
        else{ui = 0;}

        ui->setupUi(this);
        ui->lcdNumber->display(QString::number(this->current[0]*1000+this->current[1]*100+this->current[2]*10+this->current[3]).rightJustified(4, '0'));
        ui->lcdNumber_2->display(QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0'));
        mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal, &this->getLidar);

        this->m_pressure_sensor = new QPressureSensor();
        connect(m_pressure_sensor, SIGNAL(readingChanged()), this, SLOT(onPressureReadingChanged()));
        m_pressure_sensor->start();

        this->m_rotation_sensor = new QRotationSensor();
        connect(m_rotation_sensor, SIGNAL(readingChanged()), this, SLOT(onRotationReadingChanged()));
        m_rotation_sensor->start();

        ui->plainTextEdit->appendPlainText("Transponder 200-UAV v1.01a");

    }
    setmode(0);

    mysocket->doConnect();

    m_timer.start();
    m_Clock = new QTimer(this);
    m_Clock->setSingleShot(false);
    connect(m_Clock, SIGNAL(timeout()), this, SLOT(doClock()));
    m_Clock->start(1000);


    timerAlt = new QTimer(this);
    timerAlt->setSingleShot(false);
    connect(timerAlt, SIGNAL(timeout()), this, SLOT(doAlt()));
    timerAlt->start(5000);

    timerPing = new QTimer(this);
    timerPing->setSingleShot(false);
    connect(timerPing, SIGNAL(timeout()), this, SLOT(reset_ping()));

    //use a timer to allow the constructor to exit
    series = new QSplineSeries();
    series->setName("spline");
    srand((unsigned)time(NULL));

    timerpaint = new QTimer(this);
    timerpaint->setSingleShot(false);
    connect(timerpaint, SIGNAL(timeout()), this, SLOT(doPaint()));
    timerpaint->start(100);
}

void MainWindow::doClock()
{
    ui->timeEdit->setDateTime(QDateTime::currentDateTime());

    if(m_takeoff){
        ui->timeEdit_2->setTime(QTime::fromMSecsSinceStartOfDay(m_timer.elapsed()));
    }

    qDebug() << "Phone's P  sensor = " << (m_pressure_reader->pressure());
}

void MainWindow::onPressureReadingChanged()
{
    static bool first = true;

    m_pressure_reader = m_pressure_sensor->reading();

    if(first){
        first = false;
        m_alt = m_pressure_reader->pressure();
    }
    qDebug() << "Phone's 'Pa' sensor = " << m_pressure_reader->pressure();

    // If more than 1mb change then we takeoff...
    if(abs(m_alt-(m_pressure_reader->pressure())) > 100){
        m_takeoff = true;
    }

}

void MainWindow::onRotationReadingChanged()
{
    m_rotation_reader = m_rotation_sensor->reading();

    if(m_first){
        m_first = false;
        m_offset = m_rotation_reader->y();
    }

//    qDebug() << "Phone's X  rotation = " << (m_rotation_reader->x());
 //   qDebug() << "Phone's Y  rotation = " << (m_rotation_reader->y());
 //   qDebug() << "Phone's Y2 rotation = " << (m_offset);
//    qDebug() << "Phone's Z rotation = " << (m_rotation_reader->z());

    QGraphicsScene * m_graphScen = new QGraphicsScene;
    QSize x = ui->graphicsView->size();

    m_graphScen->setSceneRect(0,0,x.width(),x.height());
    float x1 = cos(m_rotation_reader->x()/(180.0/3.1415));
    float y1 = sin(m_rotation_reader->x()/(180.0/3.1415));

    int offset = m_rotation_reader->y()-m_offset;
    if (offset > 180) offset = offset -180;
    offset = offset * (x.height()/180.0);

    // -----------------------------------------
    m_graphScen->addLine((int)((x.width() /2)-(x1*70)),
                         (int)((x.height()/2)-(y1*70))+offset,
                         (int)((x.width() /2)+(x1*70)),
                         (int)((x.height()/2)+(y1*70))+offset,
                         QPen(QBrush(Qt::yellow),3));

    // -----------------------------------------
    m_graphScen->addLine((int)0,
                         (int)((x.height()/2)-0),
                         (int)x.width(),
                         (int)((x.height()/2)+0),
                         QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)(x.width() /2),
                         (int)((x.height()/2)-60),
                         (int)(x.width() /2),
                         (int)((x.height()/2)+60),
                         QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));

    // -----------------------------------------
    // Center main angle...
    m_graphScen->addLine((int)((x.width() /2)-(x1*30)),
                         (int)((x.height()/2)-(y1*30)),
                         (int)((x.width() /2)+(x1*30)),
                         (int)((x.height()/2)+(y1*30)),
                         QPen(QBrush(Qt::white),1,Qt::PenStyle(Qt::DashLine)));

    // First upper bar...
    float p_loc_A = (x.width() /2)  + (y1*15.0);
    float p_loc_B = (x.height() /2) - (x1*15.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*20)),
                         (int)(p_loc_B-(y1*20)),
                         (int)(p_loc_A+(x1*20)),
                         (int)(p_loc_B+(y1*20)),
                         QPen(QBrush(Qt::white),1,Qt::PenStyle(Qt::DashLine)));

    // Second upper bar...
    p_loc_A = (x.width() /2)  + (y1*30.0);
    p_loc_B = (x.height() /2) - (x1*30.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*10)),
                         (int)(p_loc_B-(y1*10)),
                         (int)(p_loc_A+(x1*10)),
                         (int)(p_loc_B+(y1*10)),
                         QPen(QBrush(Qt::white),1,Qt::PenStyle(Qt::DashLine)));


    // First lower bar...
    p_loc_A = (x.width() /2)  - (y1*15.0);
    p_loc_B = (x.height() /2) + (x1*15.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*20)),
                         (int)(p_loc_B-(y1*20)),
                         (int)(p_loc_A+(x1*20)),
                         (int)(p_loc_B+(y1*20)),
                         QPen(QBrush(Qt::white),1,Qt::PenStyle(Qt::DashLine)));

    // Second lower bar...
    p_loc_A = (x.width() /2)  - (y1*30.0);
    p_loc_B = (x.height() /2) + (x1*30.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*10)),
                         (int)(p_loc_B-(y1*10)),
                         (int)(p_loc_A+(x1*10)),
                         (int)(p_loc_B+(y1*10)),
                         QPen(QBrush(Qt::white),1,Qt::PenStyle(Qt::DashLine)));

    // -----------------------------------------
    m_graphScen->addLine((int)((x.width() /2)-70),
                         (int)((x.height()/2)-14+offset),
                         (int)((x.width() /2)-50),
                         (int)((x.height()/2)-11+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+70),
                         (int)((x.height()/2)-14+offset),
                         (int)((x.width() /2)+50),
                         (int)((x.height()/2)-11+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-70),
                         (int)((x.height()/2)+14+offset),
                         (int)((x.width() /2)-50),
                         (int)((x.height()/2)+11+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+70),
                         (int)((x.height()/2)+14+offset),
                         (int)((x.width() /2)+50),
                         (int)((x.height()/2)+11+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+60),
                         (int)((x.height()/2)+30+offset),
                         (int)((x.width() /2)+45),
                         (int)((x.height()/2)+21+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-60),
                         (int)((x.height()/2)+30+offset),
                         (int)((x.width() /2)-45),
                         (int)((x.height()/2)+21+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+60),
                         (int)((x.height()/2)-30+offset),
                         (int)((x.width() /2)+45),
                         (int)((x.height()/2)-21+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-60),
                         (int)((x.height()/2)-30+offset),
                         (int)((x.width() /2)-45),
                         (int)((x.height()/2)-21+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-48),
                         (int)((x.height()/2)-45+offset),
                         (int)((x.width() /2)-36),
                         (int)((x.height()/2)-32+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+48),
                         (int)((x.height()/2)-45+offset),
                         (int)((x.width() /2)+36),
                         (int)((x.height()/2)-32+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+48),
                         (int)((x.height()/2)+45+offset),
                         (int)((x.width() /2)+36),
                         (int)((x.height()/2)+32+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-48),
                         (int)((x.height()/2)+45+offset),
                         (int)((x.width() /2)-36),
                         (int)((x.height()/2)+32+offset), QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

    // -----------------------------------------
    m_graphScen->addRect(QRect(1,1,x.width()-4,x.height()-4),QPen(QBrush(Qt::gray),1));

    ui->graphicsView->setScene(m_graphScen);
    ui->graphicsView->show();

    ui->roll->setText(QString("%1").arg(abs(m_rotation_reader->x()), 0, 'f', 1));
    ui->pitch->setText(QString("%1").arg(-1*(m_rotation_reader->y()-m_offset), 0, 'f', 1));

    m_reading|=0x04;
}

void MainWindow::reset_ping()
{
    QString x;
    x = ui->pushButton_10->styleSheet();
    x.replace(QString("1 #090"), QString("1 #900"));
    ui->pushButton_10->setStyleSheet(x);
    ui->pushButton_10->update();
}

std::variant<Ui::MainWindow_port_small, Ui::MainWindow_port, Ui::MainWindow_small,Ui::MainWindow >MainWindow::getauto()
{
    static std::variant < Ui::MainWindow_port_small, Ui::MainWindow_port, Ui::MainWindow_small,Ui::MainWindow > xxz3;
    QScreen *s = QGuiApplication::primaryScreen();
    QSizeF x = s->physicalSize();
    float Ssize = sqrt(( x.rheight() * x.rheight() ) + ( x.rwidth()* x.rwidth())) / 25.4;
    //   int index = xxz3.index();
    fprintf(stderr,"Size:: %f",Ssize);

    if(s->isPortrait(s->orientation()))
    {
        if( Ssize < 7.0)    xxz3 = *new (Ui::MainWindow_port_small);
        else                xxz3 = *new (Ui::MainWindow_port);
    }
    else{
        if( Ssize < 7.0)    xxz3 = *new (Ui::MainWindow_small);
        else                xxz3 = *new (Ui::MainWindow);
    }
    this->xxz2 = &xxz3;
    return xxz3;
}

MainWindow::~MainWindow()
{
//    delete ui;
}

void MainWindow::doAlt()
{
    QString x;
    x = ui->pushButton_11->styleSheet();
    if ( alt_receiced == false){
        x.replace(QString("1 #090"), QString("1 #900"));
    }else{
        x.replace(QString("1 #900"), QString("1 #090"));
    }
    ui->pushButton_11->setStyleSheet(x);
    ui->pushButton_11->update();
    alt_receiced = false;
}

void MainWindow::setalt(int alt_mode)
{
    this->alt_mode = alt_mode;
    qDebug() << "Set ALT: " << alt_mode;
}


void MainWindow::getLidar(QByteArray array)
{
    Ui::MainWindow_small *local_ui;
    if (const auto intPtr (std::get_if<Ui::MainWindow_port_small>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else if  (const auto intPtr (std::get_if<Ui::MainWindow_port>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else if  (const auto intPtr (std::get_if<Ui::MainWindow_small>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else if  (const auto intPtr (std::get_if<Ui::MainWindow>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else return;

    local_ui->plainTextEdit->appendPlainText(array);
}

void MainWindow::getVal(QByteArray array)
{
    static char buffer[30];
    static int pos = 0;
    qDebug() << "Reseive: " << array;

    Ui::MainWindow_small *local_ui;
    if (const auto intPtr (std::get_if<Ui::MainWindow_port_small>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else if  (const auto intPtr (std::get_if<Ui::MainWindow_port>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else if  (const auto intPtr (std::get_if<Ui::MainWindow_small>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}
    else if  (const auto intPtr (std::get_if<Ui::MainWindow>(saved_this->xxz2)); intPtr) {local_ui = (Ui::MainWindow_small *)intPtr;}

    for(int i=0; i < array.size();i++){
        if(array.at(i) == '*'){
            QString x;
            x = local_ui->pushButton_10->styleSheet();
            x.replace(QString("1 #900"), QString("1 #090"));
            local_ui->pushButton_10->setStyleSheet(x);
            local_ui->pushButton_10->update();
            saved_this->timerPing->start(5000); // Turn off in 5 sec...
        }

        if(array.at(i) < 0x1F || pos >= (int)sizeof(buffer)){
            //    qDebug() << "R: >>buffer;  //m_message);

            if(pos >= 3){
                // Make the flash activity...
                {
                    static bool f = false;
                    QString x,y;
                    x = local_ui->pushButton_14->styleSheet();
                    y = x;

                    if(f == false)
                    {
                        f = true;
                        x.replace(QString("1 #0F0"), QString("1 #00F"));
                    }
                    else{
                        f = false;
                        x.replace(QString("1 #00F"), QString("1 #0F0"));
                    }

                    local_ui->pushButton_14->setStyleSheet(x);
                    local_ui->pushButton_14->update();
                    local_ui->pushButton_15->setStyleSheet(y);
                    local_ui->pushButton_15->update();
                }

                switch (buffer[0]){
                case 's':
                {
                    QString x;
                    x = local_ui->pushButton_off->styleSheet();
                    x.replace(QString("1 #561"), QString("1 #888"));
                    local_ui->pushButton_off->setStyleSheet(x);
                    local_ui->pushButton_norm->setStyleSheet(x);
                    local_ui->pushButton_stby->setStyleSheet(x);
                    local_ui->pushButton_alt->setStyleSheet(x);
                    x.replace(QString("1 #888"), QString("1 #2F0"));

                    switch(buffer[2]){
                    case 'o':
                        local_ui->pushButton_off->setStyleSheet(x);
                        saved_this->mode = 0;
                        break;

                    case 't':
                        local_ui->pushButton_stby->setStyleSheet(x);
                        saved_this->mode = 1;
                        break;

                    case 'a':
                        local_ui->pushButton_norm->setStyleSheet(x);
                        saved_this->mode = 2;
                        break;

                    case 'c':
                        local_ui->pushButton_alt->setStyleSheet(x);
                        saved_this->mode = 3;
                        break;

                    }
                    break;
                }
                case 'r':
                {
                    bool state=true;
                    if (buffer[2] == 'N') state = false;

                    QString x = tr("Annunciator %1").arg(state);

                    // ... Annunciator
                    // local_ui->plainTextEdit->appendPlainText(x);
                    break;
                }
                case 'i':
                {
                    bool state;
                    if (buffer[2] == '1') state = false;
                    else state = true;

                    QString x;
                    x = local_ui->pushButton_Ident->styleSheet();

                    if ( state == false){
                        x.replace(QString("1 #561"), QString("1 #953"));
                    }else{
                        x.replace(QString("1 #953"), QString("1 #561"));
                    }
                    local_ui->pushButton_Ident->setStyleSheet(x);
                    local_ui->pushButton_Ident->update();
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

                    local_ui->lcdNumber->display(QString::number( saved_this->current[0]*1000+
                                                               saved_this->current[1]*100+
                                                               saved_this->current[2]*10+
                                                               saved_this->current[3]).rightJustified(4, '0'));

                    break;

                }
                case 'a':
                {
                    float number;
                    char numout[10];
                    int num;
                    sscanf(buffer,"a=%f",&number);
                    QString altType="Alt.Ft.";

                    if(saved_this->alt_mode == 1){
                        for (unsigned long key=0; key < strlen(buffer); key++) if(buffer[key]=='M') {number*=3.2808399; break;}
                        num = round(number/100.0)*100;
                    }
                    else{
                        for (unsigned long key=0; key < strlen(buffer); key++) if(buffer[key]=='F') {number/=3.2808399; break;}
                        num = round(number);
                        altType="Alt.M.";
                    }
                    sprintf(numout,"%.4d",num);
                    local_ui->lcdNumber_3->display(numout);
                    local_ui->label_2->setText(altType);

                    saved_this->alt_receiced = true;
                    break;

                }
                case 'z':
                {
                    qDebug() << "T: %s\r\n" << &buffer[2];
                    local_ui->plainTextEdit->appendPlainText(&buffer[2]);
                    break;
                }

                case 'p':
                {
                    qDebug() << "P: %s\r\n" << &buffer[2];

                    bool state=true;
                    if (buffer[2] == '1') state = false;

                    QString x = tr("Hardware test status: %1").arg(state);
                    local_ui->plainTextEdit->appendPlainText(x);

                    // ...
                    if(state == true)
                    {
                        x = local_ui->pushButton_10->styleSheet();
                        x.replace(QString("1 #900"), QString("1 #090"));
                        local_ui->pushButton_10->setStyleSheet(x);
                        local_ui->pushButton_10->update();
                        saved_this->timerPing->start(5000); // Turn off in 5 sec...
                    }
                    else
                    {
                        x = local_ui->pushButton_10->styleSheet();
                        x.replace(QString("1 #090"), QString("1 #900"));
                        local_ui->pushButton_10->setStyleSheet(x);
                        local_ui->pushButton_10->update();
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
    qDebug() << "Set val";
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
    ui->lcdNumber_2->display( num);
}

void MainWindow::setmode(int m)
{
    if(mysocket != NULL)
    {
        switch(m){
        case 0: mysocket->readyWrite((char*)"s=?\r\n"); break;
        case 1: mysocket->readyWrite((char*)"s=t\r\n"); break;
        case 2: mysocket->readyWrite((char*)"s=a\r\n"); break;
        case 3: mysocket->readyWrite((char*)"s=c\r\n"); break;
        }

        // This is sendt by the micro controller...
        //        if(m==2 || m==3){
        //            mysocket->readyWrite("r=y\r\n");
        //        }
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
/*
    this->next[3]=7;
    this->next[2]=0;
    this->next[1]=0;
    this->next[0]=0;
    */
}

void MainWindow::on_pushButton_18_clicked()
{
    //   device.startDeviceDiscovery();
    //    view.show();

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost){
            qDebug() << address.toString();
            ui->plainTextEdit->appendPlainText(address.toString());
        }
    }
}


void MainWindow::on_pushButton_Ident_clicked()
{
    static bool state = false;

    if ( state == false){
        state = true;
        mysocket->readyWrite((char*)"i=1\r\n");
    }else{
        state = false;
        mysocket->readyWrite((char*)"i=0\r\n");
    }

}

// Set mode...
void MainWindow::on_pushButton_stby_clicked(){setmode(1);}
void MainWindow::on_pushButton_norm_clicked(){setmode(2);}
void MainWindow::on_pushButton_alt_clicked(){setmode(3);}

void MainWindow::on_pushButton_12_clicked(){setalt(0);}
void MainWindow::on_pushButton_13_clicked(){setalt(1);}

void MainWindow::on_pushButton_off_clicked(){
    mysocket->readyWrite((char*)"p=?\r\n");
}


void MainWindow::on_pushButton_20_clicked()
{
    m_first = true;
}

