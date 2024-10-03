#include <QtCore/QLoggingCategory>
#include <QQmlContext>
#include <QGuiApplication>
#include <QColorDialog>
#include <QNetworkInterface>
#include <QTimer>
//#include <iostream>
#include <cstdio>
#include <QApplication>
#include <QElapsedTimer>
//#include <QAmbientTemperatureReading>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QQuickWidget>
#include <QQmlProperty>
#include <QtQuick>
#include <QVideoWidget>
#include <QCameraDevice>
#include <QPixmap>
#include <QMediaRecorder>

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

#include "mainwindow.h"
#include "mytcpsocket.h"


static MainWindow *saved_this;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    saved_this = this;
    KeepAwakeHelper helper;

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
    }
//    if (ScreenMode == Qt::InvertedPortraitOrientation) ScreenMode = Qt::PortraitOrientation;

    ui->setupUi(this);
    ui->quickWidget->setSource(QUrl("qrc:/places_map.qml"));
    ui->quickWidget->rootObject()->setProperty("zoomLevel", 18);

    ui->lcdNumber->display(QString::number(this->current[0]*1000+this->current[1]*100+this->current[2]*10+this->current[3]).rightJustified(4, '0'));
    ui->lcdNumber_2->display(QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0'));

    mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal, &this->getLidar);

// Whenever the location data source signals that the current
// position is updated, the positionUpdated function is called.
#ifdef Q_OS_ANDROID
    this->m_geoPositionInfo = QGeoPositionInfoSource::createDefaultSource(this);
    if (this->m_geoPositionInfo)
    {
        connect(m_geoPositionInfo,SIGNAL(positionUpdated(QGeoPositionInfo)),this,SLOT(positionUpdated(QGeoPositionInfo)));
        // Start listening for position updates
        m_geoPositionInfo->setUpdateInterval(200);
        m_geoPositionInfo->startUpdates();
    }

    this->m_pressure_sensor = new QPressureSensor();
    connect(m_pressure_sensor, SIGNAL(readingChanged()), this, SLOT(onPressureReadingChanged()));
    m_pressure_sensor->start();

    QList<QSensor*> mySensorList;
    for (const QByteArray &type : QSensor::sensorTypes()) {
        qDebug() << "Found a sensor type:" << type;
        for (const QByteArray &identifier : QSensor::sensorsForType(type)) {
            qDebug() << "    " << "Found a sensor of that type:" << identifier;
            QSensor* sensor = new QSensor(type, this);
            sensor->setIdentifier(identifier);
            mySensorList.append(sensor);
        }
    }
#endif

    ui->plainTextEdit->appendPlainText("Transponder 200-UAV v1.01a");
    ui->listView->raise();
    ui->listView->setHidden(true);

    // Setup the parameters...
    {
        QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + RADIO);
        if (l_file->open(QIODevice::ReadOnly)){
            QString blob = l_file->readAll();
            l_file->close();
            qDebug() << blob;
            ui->textEdit->insertPlainText(blob);

        }
        else{
            set_default_radio();
        }
    }
    {
        QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + AIRPLANE);
        if (l_file->open(QIODevice::ReadOnly)){
            QString blob = l_file->readAll();
            l_file->close();
            qDebug() << blob;
            ui->textEdit_2->insertPlainText(blob);

        }
        else{
            set_default_planes();
        }
    }

    qDebug() << "  setmode  ";
    setmode(0);

    qDebug() << "  m_timer  ";
    m_timer.start();

    m_IMU = new QTimer(this);
    m_IMU->setSingleShot(false);
    connect(m_IMU, SIGNAL(timeout()), this, SLOT(onRotationReadingChanged()));
    m_IMU->start(180);

    m_Clock = new QTimer(this);
    m_Clock->setSingleShot(false);
    connect(m_Clock, SIGNAL(timeout()), this, SLOT(doClock()));
    m_Clock->start(1000);

    qDebug() << "  timerAlt  ";
    timerAlt = new QTimer(this);
    timerAlt->setSingleShot(false);
    connect(timerAlt, SIGNAL(timeout()), this, SLOT(doCheck()));
    timerAlt->start(5000);

    qDebug() << "  timerPing  ";
    timerPing = new QTimer(this);
    timerPing->setSingleShot(false);
    connect(timerPing, SIGNAL(timeout()), this, SLOT(reset_ping()));

    qDebug() << "  timerActive ";
    timerActive = new QTimer(this);
    timerActive->setSingleShot(false);
    connect(timerActive, SIGNAL(timeout()), this, SLOT(active_ping()));

    qDebug() << "  timerPictureActive ";
    timertakePicture = new QTimer(this);
    timertakePicture->setSingleShot(false);
    connect(timertakePicture, SIGNAL(timeout()), this, SLOT(takePicture()));

    /*
    QWebView *webView = new QWebView(ui->webwidget);
    webView->resize(1000,500);
    webView->move(10,10);
    QString gMapURL = "England"; // this is where you want to point
    gMapURL = "http://maps.google.com.sg/maps?q="+gMapURL+"&oe=utf-8&rls=org.mozilla:en-US:official&client=firefox-a&um=1&ie=UTF-8&hl=en&sa=N&tab=wl";
    webView->setUrl(QUrl(gMapURL));
*/


    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + "/log.txt");
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
        QString data = "System booted at: "+QDateTime::currentDateTime().toString()+"\n\r";
        l_file->write(data.toLocal8Bit());
        l_file->close();
        qDebug() << data;
        qDebug() << "Log file opened at: " << QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] <<  "/log.txt \n\r";
    }
    else{
        qDebug() << "Log file error...  ";

    }

    // try to actually initialize camera & mic
    init();

    qDebug() << "  Booted...  ";


}

MainWindow::~MainWindow()
{
    //    delete ui;
    qDebug() << "Exiting...";
    qApp->closeAllWindows();
}


void MainWindow::init()
{

#if QT_CONFIG(permissions)
    // camera
    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(cameraPermission, this, &MainWindow::init);
        return;
    case Qt::PermissionStatus::Denied:
        qWarning("Camera permission is not granted!");
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }
#endif

    // Camera devices:
    videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &MainWindow::updateCameras);
    const QRect *m_vsize = &ui->viewfinder->geometry();
    m_size = new QSize( m_vsize->width(),m_vsize->height());
    qDebug() << "TERJE:::::::" <<  m_vsize->height() << "    "  <<  m_vsize->width();

 //   setCamera(QMediaDevices::defaultVideoInput());
}


void MainWindow::updateCameras()
{
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    for (const QCameraDevice &cameraDevice : availableCameras) {
        QAction *videoDeviceAction = new QAction(cameraDevice.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
            videoDeviceAction->setChecked(true);
    }
}

void MainWindow::setCamera(const QCameraDevice &cameraDevice)
{
    m_cameraDevic = new QCameraDevice(cameraDevice);
    m_camera.reset(new QCamera(*m_cameraDevic));
    m_captureSession.setCamera(m_camera.data());

/*
    if (!m_imageCapture) {
        m_imageCapture.reset(new QImageCapture);
        m_captureSession.setImageCapture(m_imageCapture.get());
        connect(m_imageCapture.get(), &QImageCapture::readyForCaptureChanged, this,&Camera::readyForCapture);
        connect(m_imageCapture.get(), &QImageCapture::imageCaptured, this,&Camera::processCapturedImage);
        connect(m_imageCapture.get(), &QImageCapture::imageSaved, this, &Camera::imageSaved);
        connect(m_imageCapture.get(), &QImageCapture::errorOccurred, this,&Camera::displayCaptureError);
    }
*/
    m_captureSession.setVideoOutput(ui->viewfinder);
    ui->viewfinder->resize(*m_size);
    ui->viewfinder->show();


 //   updateCameraActive(m_camera->isActive());
//    updateRecorderState(m_mediaRecorder->recorderState());
//    readyForCapture(m_imageCapture->isReadyForCapture());

    m_camera->start();
}

void MainWindow::hideCamera()
{
    ui->viewfinder->resize(QSize(1,1));

    m_camera->stop();
    ui->viewfinder->hide();
    m_captureSession.disconnect();
    m_camera.reset();
}

void MainWindow::logTakeoff()
{
    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + "/flightlog.txt");
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
        QString data = "Takeoff at: "+QDateTime::currentDateTime().toString()+"\n\r";
        l_file->write(data.toLocal8Bit());
        l_file->close();
        ui->listView->appendPlainText(data);
    }
}

void MainWindow::logLanded()
{
    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + "/log.txt");
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
        QString data = "Landed at: "+QDateTime::currentDateTime().toString()+"\n\r";
        l_file->write(data.toLocal8Bit());
        l_file->close();
        ui->listView->appendPlainText(data);
        // }
    }
}

void MainWindow::doClock()
{
    static int tim = 0;

    if(!(++tim % 10)){
        static int lastPower = -1;
        if(lastPower != mysocket->Electricity){
            ui->plainTextEdit->appendPlainText("IMU Power: "+  QString::number(mysocket->Electricity));
            lastPower = mysocket->Electricity;
        }
    }

    ui->timeEdit->setDateTime(QDateTime::currentDateTime());

    // If we are in the air...
    if(m_takeoff){
        ui->timeEdit_2->setTime(QTime::fromMSecsSinceStartOfDay(m_timer.elapsed()));
        // Have we landed...
        if(this->m_speed <= 5.0){
            m_takeoff = false;
            logLanded();
        }
    }
    else{
        // If more than 40Km/t we are taking off...
        if( this->m_speed > 30.0 && this->m_speed < 500.0 ){
            m_takeoff = true;
            logTakeoff();
        }
    }
}

void MainWindow::positionUpdated(QGeoPositionInfo geoPositionInfo)
{
    static bool first = true;

//    qDebug() << "  positionUpdated  ";

    if (geoPositionInfo.isValid())
    {
        //locationDataSource->stopUpdates();
        QGeoCoordinate geoCoordinate = geoPositionInfo.coordinate();
        this->m_speed     = geoPositionInfo.attribute(QGeoPositionInfo::GroundSpeed)*3.6;
        this->m_latitude  = geoCoordinate.latitude();
        this->m_longitude = geoCoordinate.longitude();
        this->m_altitude  = geoCoordinate.altitude();
    //    this->m_head      = geoPositionInfo.attribute(QGeoPositionInfo::Direction);

        //   qDebug() << "  doClock  ";
        ui->quickWidget->rootObject()->setProperty("lat", this->m_latitude);
        ui->quickWidget->rootObject()->setProperty("lon", this->m_longitude);
/*
        qDebug() << "Lat: " << this->m_latitude << " Lon: " << this->m_longitude;
        qDebug() << "m_speed     = " << m_speed << Qt::endl;
        qDebug() << "m_latitude  = " << m_latitude << Qt::endl;
        qDebug() << "m_longitude = " << m_longitude << Qt::endl;
        qDebug() << "m_altitude  = " << m_altitude << Qt::endl;
        qDebug() << "m_head      = " << m_head << Qt::endl;
*/

        if(first){
            first = false;
            m_alt = this->m_altitude;
        }
    }
}

void MainWindow::onPressureReadingChanged()
{
//    qDebug() << "  onPressureReadingChanged  ";
    static bool first = true;

    m_pressure_reader = m_pressure_sensor->reading();

    if(first){
        first = false;
        m_alt = m_pressure_reader->pressure();
    }
    //   qDebug() << "Phone's 'Pa' sensor = " << m_pressure_reader->pressure();

    // If more than 1mb change then we takeoff...
    //    if( abs(m_alt-(m_pressure_reader->pressure())) > 100){
    //        m_takeoff = true;
    //    }

}

void MainWindow::onRotationReadingChanged()
{
    #define filterlength 15
    static double filter[filterlength]={0};

    //    qDebug() << "  onRotationReadingChanged  ";
//    m_rotation_reader = m_rotation_sensor->reading();

    double x_att = mysocket->AngleX;
    double y_att = mysocket->AngleY;
    double z_att = mysocket->AngleZ;

    double turnbank = 0;
    for(int x=0; x < filterlength-1;x++){
        filter[x] = filter[x+1];
        turnbank+=filter[x];
    }
    double f = 4 * mysocket->AccY * -75.0;
    if(f >  75.0)f =  75.0;
    if(f < -75.0)f = -75.0;
    filter[filterlength-1] = f;
    turnbank+=filter[filterlength-1];
    turnbank/=filterlength;

    /*
    double x_att = m_rotation_reader->x();
    double y_att = m_rotation_reader->y();
    double z_att = m_rotation_reader->z();
    */
    m_head = z_att;

    if(m_first){
        m_first = false;
        m_offset = mysocket->AngleY; // m_rotation_reader->y();
    }

    //    qDebug() << "Phone's X  rotation = " << (m_rotation_reader->x());
    //   qDebug() << "Phone's Y  rotation = " << (m_rotation_reader->y());
    //   qDebug() << "Phone's Y2 rotation = " << (m_offset);
    //    qDebug() << "Phone's Z rotation = " << (m_rotation_reader->z());

    QGraphicsScene * m_graphScen = new QGraphicsScene;
    QSize x = ui->graphicsView->size();

    m_graphScen->setSceneRect(0,0,x.width(),x.height());
    float x1 = cos(x_att/(180.0/3.1415));
    float y1 = sin(x_att/(180.0/3.1415));

    int offset = (y_att/2)-(m_offset/2);
    if (offset > 180) offset = offset -180;
    offset = offset * (x.height()/180.0);

    // -----------------------------------------
    // Artificial Horizon line...
    m_graphScen->addLine((int)((x.width() /2)-(x1*200)),
                         (int)((x.height()/2)-(y1*200))+offset,
                         (int)((x.width() /2)+(x1*200)),
                         (int)((x.height()/2)+(y1*200))+offset,
                         QPen(QBrush(Qt::yellow),6));

    // -----------------------------------------
    // Horisontal center line...
    m_graphScen->addLine((int)5,
                         (int)((x.height()/2)-0),
                         (int)x.width()-5,
                         (int)((x.height()/2)+0),
                         QPen(QBrush(Qt::green),2,Qt::PenStyle(Qt::DashLine)));
    // Vertical center line...
    m_graphScen->addLine((int)(x.width() /2),
                         (int) 5,
                         (int)(x.width() /2),
                         (int)x.height()-5,
                         QPen(QBrush(Qt::green),2,Qt::PenStyle(Qt::DashLine)));

    // Horisontal bottom line...
    m_graphScen->addLine((int)175,
                         (int)((x.height()/1)-100),
                         (int)x.width()-175,
                         (int)((x.height()/1)-100),
                         QPen(QBrush(Qt::darkMagenta),8));

    // Horisontal bottom line...
    m_graphScen->addEllipse(turnbank+((x.width()/2)-11),x.height()-110,20,20,QPen(Qt::white),QBrush(Qt::cyan));

    // -----------------------------------------
    // Center main angle...
    m_graphScen->addLine((int)((x.width() /2)-(x1*60)),
                         (int)((x.height()/2)-(y1*60)),
                         (int)((x.width() /2)+(x1*60)),
                         (int)((x.height()/2)+(y1*60)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // First upper bar...
    float p_loc_A = (x.width() /2)  + (y1*15.0);
    float p_loc_B = (x.height() /2) - (x1*15.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*50)),
                         (int)(p_loc_B-(y1*50)),
                         (int)(p_loc_A+(x1*50)),
                         (int)(p_loc_B+(y1*50)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // Second upper bar...
    p_loc_A = (x.width() /2)  + (y1*30.0);
    p_loc_B = (x.height() /2) - (x1*30.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*40)),
                         (int)(p_loc_B-(y1*40)),
                         (int)(p_loc_A+(x1*40)),
                         (int)(p_loc_B+(y1*40)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // Third upper bar...
    p_loc_A = (x.width() /2)  + (y1*45.0);
    p_loc_B = (x.height() /2) - (x1*45.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*30)),
                         (int)(p_loc_B-(y1*30)),
                         (int)(p_loc_A+(x1*30)),
                         (int)(p_loc_B+(y1*30)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // Forth upper bar...
    p_loc_A = (x.width() /2)  + (y1*60.0);
    p_loc_B = (x.height() /2) - (x1*60.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*20)),
                         (int)(p_loc_B-(y1*20)),
                         (int)(p_loc_A+(x1*20)),
                         (int)(p_loc_B+(y1*20)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // First lower bar...
    p_loc_A = (x.width() /2)  - (y1*15.0);
    p_loc_B = (x.height() /2) + (x1*15.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*50)),
                         (int)(p_loc_B-(y1*50)),
                         (int)(p_loc_A+(x1*50)),
                         (int)(p_loc_B+(y1*50)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // Second lower bar...
    p_loc_A = (x.width() /2)  - (y1*30.0);
    p_loc_B = (x.height() /2) + (x1*30.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*40)),
                         (int)(p_loc_B-(y1*40)),
                         (int)(p_loc_A+(x1*40)),
                         (int)(p_loc_B+(y1*40)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // Third lower bar...
    p_loc_A = (x.width() /2)  - (y1*45.0);
    p_loc_B = (x.height() /2) + (x1*45.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*30)),
                         (int)(p_loc_B-(y1*30)),
                         (int)(p_loc_A+(x1*30)),
                         (int)(p_loc_B+(y1*30)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));
    // Forth lower bar...
    p_loc_A = (x.width() /2)  - (y1*60.0);
    p_loc_B = (x.height() /2) + (x1*60.0);

    m_graphScen->addLine((int)(p_loc_A-(x1*20)),
                         (int)(p_loc_B-(y1*20)),
                         (int)(p_loc_A+(x1*20)),
                         (int)(p_loc_B+(y1*20)),
                         QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

    // -----------------------------------------

    m_graphScen->addLine((int)((x.width() /2)-100),
                         (int)((x.height()/2)-19+offset),
                         (int)((x.width() /2)-80),
                         (int)((x.height()/2)-15+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+100),
                         (int)((x.height()/2)-19+offset),
                         (int)((x.width() /2)+80),
                         (int)((x.height()/2)-15+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-100),
                         (int)((x.height()/2)+19+offset),
                         (int)((x.width() /2)-80),
                         (int)((x.height()/2)+15+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+100),
                         (int)((x.height()/2)+19+offset),
                         (int)((x.width() /2)+80),
                         (int)((x.height()/2)+15+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));



    m_graphScen->addLine((int)((x.width() /2)+80),
                         (int)((x.height()/2)+40+offset),
                         (int)((x.width() /2)+60),
                         (int)((x.height()/2)+30+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-80),
                         (int)((x.height()/2)+40+offset),
                         (int)((x.width() /2)-60),
                         (int)((x.height()/2)+30+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+80),
                         (int)((x.height()/2)-40+offset),
                         (int)((x.width() /2)+60),
                         (int)((x.height()/2)-30+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-80),
                         (int)((x.height()/2)-40+offset),
                         (int)((x.width() /2)-60),
                         (int)((x.height()/2)-30+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));



    m_graphScen->addLine((int)((x.width() /2)-60),
                         (int)((x.height()/2)-60+offset),
                         (int)((x.width() /2)-45),
                         (int)((x.height()/2)-45+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+60),
                         (int)((x.height()/2)-60+offset),
                         (int)((x.width() /2)+45),
                         (int)((x.height()/2)-45+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)+60),
                         (int)((x.height()/2)+60+offset),
                         (int)((x.width() /2)+45),
                         (int)((x.height()/2)+45+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));

    m_graphScen->addLine((int)((x.width() /2)-60),
                         (int)((x.height()/2)+60+offset),
                         (int)((x.width() /2)-45),
                         (int)((x.height()/2)+45+offset), QPen(QBrush(Qt::red),2,Qt::PenStyle(Qt::DashLine)));


    // -----------------------------------------
    m_graphScen->addRect(QRect(1,1,x.width()-4,x.height()-4),QPen(QBrush(Qt::gray),1));

    ui->graphicsView->setScene(m_graphScen);
    ui->graphicsView->show();

    if(m_speed < 300 && m_speed > 0){
        ui->speed->setText(QString("%1").arg(abs(m_speed), 0, 'f', 1));
    }
    if(mysocket->IMUconnected == true){
        ui->roll->setText(QString("%1").arg(abs(x_att), 0, 'f', 0));
        ui->pitch->setText(QString("%1").arg(-1*(y_att-m_offset), 0, 'f', 0));
        ui->temp->setText(QString("%1").arg(this->mysocket->Electricity, 0, 'f', 0));
        ui->temperature->setText(QString("%1").arg(mysocket->Temperature, 0, 'f', 1));
        ui->compass->setText(QString("%1").arg(abs(m_head), 0, 'f', 1));
    }
    if( m_altitude != 9999){
        ui->altitude->setText(QString("%1").arg(m_altitude*3.2808399, 0, 'f', 0));
    }
    m_reading|=0x04;
}

void MainWindow::reset_ping()
{
    qDebug() << "  reset_ping  ";
    QString x;
    x = ui->pushButton_10->styleSheet();
    x.replace(QString("1 #090"), QString("1 #900"));
    ui->pushButton_10->setStyleSheet(x);
    ui->pushButton_10->update();
}

void MainWindow::active_ping()
{
    qDebug() << "  reset_active  ";
    QString x;
    x = ui->pushButton_14->styleSheet();
    x.replace(QString("1 #090"), QString("1 #900"));
    ui->pushButton_14->setStyleSheet(x);
    ui->pushButton_14->update();
}

void MainWindow::doCheck()
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
    Ui::SCREEN *local_ui = (Ui::SCREEN *)&saved_this->xxz3;

    local_ui->plainTextEdit->appendPlainText(array);
}

void MainWindow::getVal(QByteArray array)
{
    static char buffer[30];
    static int pos = 0;
//    qDebug() << "Reseive: " << array;

    Ui::SCREEN *local_ui = (Ui::SCREEN *)&saved_this->xxz3;

    for(int i=0; i < array.size();i++){
        if(array.at(i) == '*'){
            QString x;
            x = local_ui->pushButton_10->styleSheet();
            x.replace(QString("1 #900"), QString("1 #090"));
            local_ui->pushButton_10->setStyleSheet(x);
            local_ui->pushButton_10->update();
            saved_this->timerPing->start(10000); // Turn off in 10 sec...
        }

        if(array.at(i) < 0x1F || pos >= (int)sizeof(buffer)){
            //    qDebug() << "R: >>buffer;  //m_message);

            if(pos >= 3){

                // Log all commands... This might be slow... will look at a timed write...
                QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + "/log.txt");
                if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
                    QString data = QDateTime::currentDateTime().toString()+": "+buffer+"\n\r";
                    l_file->write(data.toLocal8Bit());
                    l_file->close();
                }

                // Make the flash activity...
                {
                    QString x = local_ui->pushButton_14->styleSheet();
                    x.replace(QString("1 #900"), QString("1 #090"));
                    local_ui->pushButton_14->setStyleSheet(x);
                    local_ui->pushButton_14->update();
                    saved_this->timerActive->start(10000); // Turn off in 10 sec...
                }

                switch (buffer[0]){
                case 's':
                {
                    QString x;
                    x = local_ui->pushButton_off->styleSheet();
                    x.replace(QString("1 #2A0"), QString("1 #888"));
                    local_ui->pushButton_off->setStyleSheet(x);
                    local_ui->pushButton_norm->setStyleSheet(x);
                    local_ui->pushButton_stby->setStyleSheet(x);
                    local_ui->pushButton_alt->setStyleSheet(x);
                    x.replace(QString("1 #888"), QString("1 #2A0"));

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
                    if (buffer[2] == '0') state = false;
                    else state = true;

                    QString x = local_ui->pushButton_Ident->styleSheet();

                    if ( state == false){
                        x.replace(QString("1 #900"), QString("1 #888"));
                    }else{
                        x.replace(QString("1 #888"), QString("1 #900"));
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
                    snprintf(numout,5,"%.4d",number);

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
                    char numout[20];
                    int num;
                    sscanf(buffer,"a=%f",&number);
                    QString altType="Alt.Ft.";

                    if(saved_this->alt_mode == 1){
                        for (unsigned long key=0; key < strlen(buffer); key++)
                            if(buffer[key]=='M') {
                                number*=3.2808399;
                                break;
                            }
                        num = round(number/100.0)*100;
                    }
                    else{
                        for (unsigned long key=0; key < strlen(buffer); key++)
                            if(buffer[key]=='F') {
                                number/=3.2808399;
                                break;
                            }
                        num = round(number);
                        altType="Alt.M.";
                    }
                    snprintf(numout,20,"%.4d",num);
                    local_ui->lcdNumber_3->display(numout);
                    local_ui->label_2->setText(altType);
                    local_ui->baro_alt->setText(numout);

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
    qApp->closeAllWindows();
    qApp->exit();
    QCoreApplication::quit();
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
#ifndef Q_OS_IOS
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
#endif
    mode = m;
}

void MainWindow::on_pushButton_clicked(  ){addnext(1);}
void MainWindow::on_pushButton_2_clicked(){addnext(2);}
void MainWindow::on_pushButton_3_clicked(){addnext(3);}
void MainWindow::on_pushButton_4_clicked(){addnext(4);}
void MainWindow::on_pushButton_5_clicked(){addnext(5);}
void MainWindow::on_pushButton_6_clicked(){addnext(6);}
void MainWindow::on_pushButton_7_clicked(){addnext(7);}
void MainWindow::on_pushButton_8_clicked(){};//addnext(8);}
void MainWindow::on_pushButton_9_clicked(){};//addnext(9);}
void MainWindow::on_pushButton_17_clicked(){addnext(0);}

void MainWindow::on_exit_2_clicked(){           this->close();  }
void MainWindow::on_pushButton_19_clicked(){    this->close();  }

void MainWindow::on_pushButton_16_clicked()
{
    this->current[3]=this->next[3];
    this->current[2]=this->next[2];
    this->current[1]=this->next[1];
    this->current[0]=this->next[0];

    char data[100];
    snprintf(data,100,"c=%d\r\n",this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]);
    //    QString num = QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]);
    //    QString msg = QString("c=%1\r\n").arg(num);
    qDebug() << data;

#ifndef Q_OS_IOS
    mysocket->readyWrite(data);
#endif

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
/*
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost){
            qDebug() << address.toString();
            ui->plainTextEdit->appendPlainText(address.toString());
        }
    }
*/
    this->next[3]=0;
    this->next[2]=0;
    this->next[1]=0;
    this->next[0]=7;

    QString num = QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0');
    ui->lcdNumber_2->display( num);

    char data[100];
    snprintf(data,100,"c=%d\r\n",7000);
    qDebug() << data;

#ifndef Q_OS_IOS
    mysocket->readyWrite(data);
#endif
}


void MainWindow::on_pushButton_Ident_clicked()
{
#ifndef Q_OS_IOS
    mysocket->readyWrite((char*)"i=s\r\n");
#endif

}

// Set mode...
void MainWindow::on_pushButton_stby_clicked(){setmode(1);}
void MainWindow::on_pushButton_norm_clicked(){setmode(2);}
void MainWindow::on_pushButton_alt_clicked(){setmode(3);}

void MainWindow::on_pushButton_12_clicked(){setalt(0);}
void MainWindow::on_pushButton_13_clicked(){setalt(1);}

void MainWindow::on_pushButton_off_clicked(){
#ifndef Q_OS_IOS
    mysocket->readyWrite((char*)"p=?\r\n");
#endif
}

// Is 4
void MainWindow::on_select_gyro_page_clicked()          {ui->stackedWidget->setCurrentIndex(0);}
//void MainWindow::on_select_dumy_page_clicked()          {ui->viewfinder->show(); ui->stackedWidget->setCurrentIndex(3);}
void MainWindow::on_select_dumy_page_clicked()          {setCamera(QMediaDevices::defaultVideoInput()); ui->stackedWidget->setCurrentIndex(3);}

// Is 0
void MainWindow::on_select_dumy_page2_clicked()         {ui->stackedWidget->setCurrentIndex(1);}
void MainWindow::on_select_transponder_page_clicked()   {
    if( mysocket->Transponderstat == "true"){
        ui->stackedWidget->setCurrentIndex(4);
    }
    else{
        setCamera(QMediaDevices::defaultVideoInput());
        ui->stackedWidget->setCurrentIndex(3);
    }
}

// Is 1
void MainWindow::on_select_transponder_page2_clicked()  {ui->stackedWidget->setCurrentIndex(2);}
void MainWindow::on_select_gyro_page2_clicked()         {ui->stackedWidget->setCurrentIndex(0);}

// Is 2
//void MainWindow::on_select_transponder_page2_2_clicked(){ui->viewfinder->show(); ui->stackedWidget->setCurrentIndex(3);}
void MainWindow::on_select_transponder_page2_2_clicked(){setCamera(QMediaDevices::defaultVideoInput()); ui->stackedWidget->setCurrentIndex(3);}
void MainWindow::on_select_gyro_page2_2_clicked()       {ui->stackedWidget->setCurrentIndex(1);}

// Is 3
//void MainWindow::on_select_transponder_page2_3_clicked(){ui->viewfinder->hide(); ui->stackedWidget->setCurrentIndex(4);}
//void MainWindow::on_select_gyro_page2_3_clicked()       {ui->viewfinder->hide(); ui->stackedWidget->setCurrentIndex(2);}
void MainWindow::on_select_transponder_page2_3_clicked(){
    if( mysocket->Transponderstat == "true"){
        hideCamera(); ui->stackedWidget->setCurrentIndex(4);
    }
    else{
        hideCamera(); ui->stackedWidget->setCurrentIndex(0);
    }
}
void MainWindow::on_select_gyro_page2_3_clicked()
{
    hideCamera();
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_imu_reset_clicked()
{
    m_first = true;
    ui->imu_reset->setIcon(QIcon(":/icons/..."));
    ui->imu_reset->setIconSize(QSize(65, 65));
}


void MainWindow::on_timer_start_clicked()
{
    QString x = ui->timer_start->styleSheet();
    if(m_armed == false)
    {
        m_armed = true;
        x.replace(QString("1 #888"), QString("1 #00F"));
        qDebug() << "Timer pressed 1...";
        ui->listView->setHidden(false);
    }
    else{
        m_armed = false;
        x.replace(QString("1 #00F"), QString("1 #888"));
        qDebug() << "Timer pressed 2...";
        ui->listView->setHidden(true);
    }

    ui->timer_start->setStyleSheet(x);
    ui->timer_start->update();
}


void MainWindow::on_textEdit_textChanged()
{
    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + RADIO);
    if( l_file->open(QIODevice::ReadWrite )){
        QString data = ui->textEdit->toPlainText();
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }
}


void MainWindow::on_textEdit_2_textChanged()
{
    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + AIRPLANE);
    if( l_file->open(QIODevice::ReadWrite )){
        QString data = ui->textEdit_2->toPlainText();
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }

}

void MainWindow::set_default_radio()
{
    ui->textEdit->setText("121.500\tEmergency\n");
    ui->textEdit->insertPlainText("123.500\tStandard\n");
    ui->textEdit->insertPlainText("122.500\tNAK\n");
    ui->textEdit->insertPlainText("123.450\tPlane to Plane\n");
    ui->textEdit->insertPlainText("119.100\tKjeller\n");
    ui->textEdit->insertPlainText("122.700\tTorsnes\n");
    ui->textEdit->insertPlainText("118.470\tOslo Approache\n");


    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[QStandardPaths::LocateFile] + RADIO);
    if( l_file->open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QString data = ui->textEdit->toPlainText();
        qDebug() << data;
        ui->plainTextEdit->appendPlainText(data);
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }
}

void MainWindow::set_default_planes()
{
    ui->textEdit_2->setText("LN-YKQ\tRans S6\t\tSam S.\n");
    ui->textEdit_2->insertPlainText("LN-YYX\tAeros 2\t\tTerje\n");
    ui->textEdit_2->insertPlainText("LN-YXY\tDelta-jet TL22\tTerje\n");
    ui->textEdit_2->insertPlainText("LN-YPE\tAir Creation\tHarald\n");
    ui->textEdit_2->insertPlainText("LN-YRI\tPeregrin\t\tRingerike\n");
    ui->textEdit_2->insertPlainText("LN-YRM\tAeroprakt\tRingerike\n");
    ui->textEdit_2->insertPlainText("LN-YZG\tIcarus\t\tMorten\n");
    ui->textEdit_2->insertPlainText("LN-YYV\tIcarus\t\tTerje S.\n");
    ui->textEdit_2->insertPlainText("LN-YPL\tIcarus\t\tOlaf\n");
    ui->textEdit_2->insertPlainText("LN-YOG\tSamba\t\tJan Ove\n");
    ui->textEdit_2->insertPlainText("LN-YRY\tRans S6S\tAlf Nipe\n");


    QFile *l_file = new QFile(QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[QStandardPaths::LocateFile] + AIRPLANE);
    if( l_file->open(QIODevice::WriteOnly | QIODevice::Truncate)){
        QString data = ui->textEdit_2->toPlainText();
        qDebug() << data;
        ui->plainTextEdit->appendPlainText(data);
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }
}

void MainWindow::on_pushButton_15_clicked()
{
    set_default_radio();
    set_default_planes();

}

void MainWindow::on_reconnect_clicked()
{
    delete(mysocket);
    mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal, &this->getLidar);
}

void MainWindow::on_pushButton_20_clicked()
{
    QDateTime date = QDateTime::currentDateTime();
//    QString filename = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + "/"+date.toString("yyyy.dd.MM.hh.mm.ss").append(".jpg");
    QString filename = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0] + "/"+date.toString("yyyy.dd.MM.hh.mm.ss").append(".mp4");

    qDebug() << "Recording ";
    m_camera->stop();

    QCamera *camera = new QCamera(*m_cameraDevic);
    m_recorder = new QMediaRecorder(camera);
//    m_recorder->setOutputLocation(QUrl(filename));
    m_recorder->setOutputLocation(filename);

    camera->start();
    m_recorder->record();
    QThread::msleep(5000);
    qDebug()<<m_recorder->error();
    m_recorder->stop();
    camera->stop();

    m_camera->start();

 /*
    QWidget *widget= ui->viewfinder;
    QPixmap pixmap(widget->size());
    pixmap.fill(Qt::transparent);
    widget->render(&pixmap);
    widget->grab().save(filename);


    QWidget *widget= ui->viewfinder;
    QPixmap pixmap(widget->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    widget->render(&painter, QPoint(), QRegion(), QWidget::DrawWindowBackground | QWidget::IgnoreMask | QWidget::DrawChildren);
 //   pixmap.save(filename);
  //  widget->grab().save(filename);

    ui->page_5->grab().save(filename);
//    ui->viewfinder->grab().save(filename);
    */
    qDebug() << "Store file: " << filename;
 }

void MainWindow::takePicture()
{
    on_pushButton_20_clicked();
}

void MainWindow::on_pushButton_21_clicked()
{
    timertakePicture->start(3000);

}


void MainWindow::on_pushButton_22_clicked()
{
    timertakePicture->start(10000);
}


void MainWindow::on_pushButton_23_clicked()
{
    timertakePicture->stop();
}


