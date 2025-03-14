#include <QtCore/QLoggingCategory>
#include <QQmlContext>
#include <QGuiApplication>
#include <QColorDialog>
#include <QNetworkInterface>
#include <QTimer>
#include <cstdio>
#include <QApplication>
#include <QElapsedTimer>
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
#include <QImageCapture>
#include <QMediaFormat>
#include <QMediaPlayer>
#include <QOrientationSensor>
#include <QImageCapture>

//***C++11 Style:***
#include <chrono>

//#include <QCameraImageCapture>


#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

// Q_IMPORT_PLUGIN(QDarwinLocationPermissionPlugin)

#include "mainwindow.h"
#include "mytcpsocket.h"

using namespace std;
using namespace std::chrono;

//static MainWindow *saved_this;

#undef USE_KeepAwakeHelper

// ----------------------------------------------
// ----------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    _widgetAI   ( Q_NULLPTR ),
    _widgetTC   ( Q_NULLPTR ),
    _widgetALT  ( Q_NULLPTR ),
    _widgetASI  ( Q_NULLPTR ),
    _widgetVSI  ( Q_NULLPTR ),
    _widgetHI   ( Q_NULLPTR ),
    _widgetEADI ( Q_NULLPTR ),
    _widgetEHSI ( Q_NULLPTR )
{
//    saved_this = this;
#if defined(Q_OS_ANDROID) && defined(USE_KeepAwakeHelper)
    static KeepAwakeHelper helper;
#endif

    //   if (ScreenMode == Qt::InvertedPortraitOrientation) ScreenMode = Qt::PortraitOrientation;

    ui->setupUi(this);

    ui->quickWidget->setSource(QUrl("qrc:/places_map.qml"));
    ui->quickWidget->rootObject()->setProperty("zoomLevel", 18);

    ui->lcdNumber->display(QString::number(this->current[0]*1000+this->current[1]*100+this->current[2]*10+this->current[3]).rightJustified(4, '0'));
    ui->lcdNumber_2->display(QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0'));
    ui->plainTextEdit->appendPlainText("Transponder 200-UAV v1.01a");
    //ui->listView->
    ui->listView->appendPlainText("Transponder 200-UAV v1.01a");
    ui->listView->raise();
    ui->listView->setHidden(true);

    m_msgBox = new QMessageBox(QMessageBox::Information,tr("Software is booting"),tr("Please wait for the system to connect!"));
    m_msgBox->setStandardButtons(QMessageBox::NoButton);
    m_msgBox->show();

    mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal, &this->setIMU);

    // Whenever the location data source signals that the current
    // position is updated, the positionUpdated function is called.
    this->m_geoPositionInfo = QGeoPositionInfoSource::createDefaultSource(this);
    if (this->m_geoPositionInfo)
    {
        connect(m_geoPositionInfo,SIGNAL(positionUpdated(QGeoPositionInfo)),this,SLOT(positionUpdated(QGeoPositionInfo)));
        // Start listening for position updates
        m_geoPositionInfo->setUpdateInterval(200);
        m_geoPositionInfo->startUpdates();
    }

    // Setup the parameters...
    {
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString(RADIO));
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
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString(AIRPLANE));
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

    QString data = "System booted at: "+QDateTime::currentDateTime().toString();
    ui->listView->appendPlainText(data);

    QFile *l_file = new QFile(QString(LOG_DIR)+ QString("flightlog.txt"));
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
        l_file->write(data.toLocal8Bit()+"\n");
        l_file->close();
    }
    else{
        qDebug() << "Log file error...  ";
    }
    // try to actually initialize camera & mic
    init();

    _widgetAI  = ui->widgetai;
    _widgetAI->reinit();
    _widgetTC  = ui->widgetTC;
    _widgetTC->reinit();
    _widgetALT  = ui->widgetALT;
    _widgetALT->reinit();
    _widgetASI  = ui->widgetASI;
    _widgetASI->reinit();
    _widgetVSI  = ui->widgetVSI;
    _widgetVSI->reinit();
    _widgetHI  = ui->widgetHI;
    _widgetHI->reinit();
    _widgetEADI  = ui->widgetEADI;
    _widgetEADI->reinit();
    _widgetEHSI  = ui->widgetEHSI;
    _widgetEHSI->reinit();

    _widgetHI->setHeading(0);
    _widgetHI->redraw();

    _widgetVSI->setClimbRate(0);
    _widgetVSI->redraw();

    _widgetASI->setAirspeed(0);
    _widgetASI->redraw();

    _widgetALT->setAltitude(0);
    _widgetALT->redraw();

    _widgetTC->setTurnRate(0);
    _widgetTC->setSlipSkid(0);
    _widgetTC->redraw();

    _widgetAI->setRoll ( 0 );
    _widgetAI->setPitch( 0 );
    _widgetAI->redraw();


    _widgetEADI->setFltMode( qfi_EADI::FltMode::Off);
    _widgetEADI->setSpdMode( qfi_EADI::SpdMode::Off);
    _widgetEADI->setLNAV( qfi_EADI::LNAV::Off);
    _widgetEADI->setVNAV( qfi_EADI::VNAV::Off);
    _widgetEADI->setRoll( 0.0 );
    _widgetEADI->setPitch( 0.0 );
    _widgetEADI->setFPM( 0.0, 0.0 );
    _widgetEADI->setSlipSkid( 0.0 );
    _widgetEADI->setTurnRate( 0.0 );
    _widgetEADI->setDots( 0.0,0.0,false,false);
    _widgetEADI->setFD( 0.0,0.0,true);
    _widgetEADI->setStall( false );
    _widgetEADI->setAltitude( 0.0 );
    _widgetEADI->setPressure( 1024.13, qfi_EADI::PressureMode::MB );
    _widgetEADI->setAirspeed( 0.0 );
    _widgetEADI->setMachNo( 0.0 );
    _widgetEADI->setHeading( 0.0 );
    _widgetEADI->setClimbRate( 0.0 );
    _widgetEADI->setAirspeedSel( 0.0 );
    _widgetEADI->setAltitudeSel(0 );
    _widgetEADI->setHeadingSel( 0.0 );
    _widgetEADI->setVfe( 0.0 );
    _widgetEADI->setVne( 0.0 );
    _widgetEADI->redraw();

    _widgetEHSI->setHeading( 45);
    _widgetEHSI->setCourse( 270 );
    _widgetEHSI->setBearing( 280, true );
    _widgetEHSI->setDeviation( 4.0, qfi_EHSI::CDI::FROM);
    _widgetEHSI->setDistance( 200, true );
    _widgetEHSI->setHeadingSel( 50 );
    _widgetEHSI->setHeading(0);
    _widgetEHSI->redraw();

    // Save the current index page...
    currentIndex = ui->stackedWidget->currentIndex();

    qDebug() << "  Booted...  " << currentIndex << " Index";

    // Quantum computer test, just to see if it would work...
    //    Qiskit();
}

MainWindow::~MainWindow()
{
    qDebug() << "Exiting...";
    qApp->closeAllWindows();
}

void MainWindow::setButtonIcon(QString iconPath, QPushButton* button)
{
    QString str("Test");
    qApp->applicationDirPath().append(iconPath);
    QPixmap pixmap(str);
    QIcon buttonIcon(pixmap);
    button->setIcon(buttonIcon);
    button->setIconSize(pixmap.rect().size());
}

// Call back funtion from the sensor handler...
void MainWindow::setIMU(void *parent, bool use_imu)
{
    MainWindow* local = (MainWindow*)parent;
    QList<QSensor*> mySensorList;

    local->m_use_imu = use_imu;
    qDebug() << "Found a sensors...";
    for (const QByteArray &type : QSensor::sensorTypes()) {
        qDebug() << "Found a sensor type:" << type;

        for (const QByteArray &identifier : QSensor::sensorsForType(type)) {
            qDebug() << "    " << "Found a sensor of that type:" << identifier;
            QSensor* sensor = new QSensor(type, local);
            sensor->setIdentifier(identifier);
            mySensorList.append(sensor);
        }

        if(!strncmp(type,"QPressureSensor",strlen("QPressureSensor"))){
            local->m_pressure_sensor = new QPressureSensor();
            connect(local->m_pressure_sensor, SIGNAL(readingChanged()), local, SLOT(onPressureReadingChanged()));
            local->m_pressure_sensor->start();
            local->m_pressure_sensor->setDataRate(4);
            qDebug() << "Found a sensor QPressureSensor";
            local->ui->radioButton_3->setChecked(true);
        }

        // If we do not have an external IMU...
        if(use_imu  == false)
        {
            if(!strncmp(type,"QRotationSensor",strlen("QRotationSensor"))){
                local->m_rotation_sensor = new QRotationSensor();
                local->m_rotation_sensor->setDataRate(40);
                connect(local->m_rotation_sensor, SIGNAL(readingChanged()), local, SLOT(onRotationReadingChanged()));
                local->m_rotation_sensor->start();
                qDebug() << "Found a sensor QRotationSensor";
            }

            if(!strncmp(type,"QOrientationSensor",strlen("QOrientationSensor"))){
                local->m_orientation_sensor = new QOrientationSensor();
                connect(local->m_orientation_sensor, SIGNAL(readingChanged()), local, SLOT(onOrientationReadingChanged()));
                local->m_orientation_sensor->start();
                qDebug() << "Found a sensor QRotationSensor";
            }

            if(!strncmp(type,"QCompass",strlen("QCompass"))){
                local->m_compass_sensor = new QCompass();
                connect(local->m_compass_sensor, SIGNAL(readingChanged()), local, SLOT(onCompassReadingChanged()));
                local->m_compass_sensor->start();
                local->m_compass_sensor->setDataRate(4);
                qDebug() << "Found a sensor QCompass";
            }

            if(!strncmp(type,"QAccelerometer",strlen("QAccelerometer"))){
                local->m_accel_sensor = new QAccelerometer();
                local->m_accel_sensor->setDataRate(40);
                connect(local->m_accel_sensor, SIGNAL(readingChanged()), local, SLOT(onAccelerometerReadingChanged()));
                local->m_accel_sensor->start();
                qDebug() << "Found a sensor QAccelerometerReading";
            }

            if(!strncmp(type,"QGyroscope",strlen("QGyroscope"))){
                local->m_gyro_sensor = new QGyroscope();
                local->m_gyro_sensor->setDataRate(40);
                connect(local->m_gyro_sensor, SIGNAL(readingChanged()), local, SLOT(onGyroReadingChanged()));
                local->m_gyro_sensor->start();
                qDebug() << "Found a sensor QGyroscopeReading";
            }

            if(!strncmp(type,"QMagnetometer",strlen("QMagnetometer"))){
                local->m_mag_sensor = new QMagnetometer();
                local->m_mag_sensor->setDataRate(40);
                connect(local->m_mag_sensor, SIGNAL(readingChanged()), local, SLOT(onMagReadingChanged()));
                local->m_mag_sensor->start();
                qDebug() << "Found a sensor QMagnetometerReading";
            }
        }
    }
    // If we do not have an external IMU...
    if(use_imu  == false)
    {
        QString data = "IMU NOT found...";
        local->ui->listView->appendPlainText(data);
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString("flightlog.txt"));
        if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
            l_file->write(data.toLocal8Bit()+"\n");
            l_file->close();
        }
    }
    else{
        QString data = "IMU found and connected...";
        local->ui->listView->appendPlainText(data);
        local->ekf.m_use_gpt = true;
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString("flightlog.txt"));
        if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
            l_file->write(data.toLocal8Bit()+"\n");
            l_file->close();
        }
    }

    qDebug() << mySensorList;

    if(local->ui->radioButton_3->isChecked() == false){
        local->ui->radioButton_3->setCheckable(false);
        local->ui->dial->hide();
        local->ui->dial_2->hide();
        local->ui->doubleSpinBox->hide();
        local->ui->doubleSpinBox_2->hide();

        QString x = local->ui->label_18->styleSheet();
        x.replace(QString("135"), QString("80"));
        local->ui->label_18->setStyleSheet(x);
        local->ui->label_18->update();

    }

    local->m_Display = new QTimer(local);
    local->m_Display->setSingleShot(false);
    connect(local->m_Display, SIGNAL(timeout()), local, SLOT(onReadingChanged()));
    local->m_Display->start(100);

    local->m_IMU = new QTimer(local);
    local->m_IMU->setSingleShot(false);
    connect(local->m_IMU, SIGNAL(timeout()), local, SLOT(EKF()));
    local->m_IMU->start(40);

    local->m_dt = QDateTime::currentMSecsSinceEpoch();
}

void MainWindow::permissionUpdated(const QPermission &permission)
{
    if (permission.status() != Qt::PermissionStatus::Granted){
        qDebug() << "Precise location permission denied";
        return;
    }
    auto locationPermission = permission.value<QLocationPermission>();
    if (!locationPermission || locationPermission->accuracy() != QLocationPermission::Precise){
        qDebug() << "Precise location permission error";
        return;
    }
    qDebug() << "Precise location OK";
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

    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);
    qApp->requestPermission(locationPermission, this, &MainWindow::permissionUpdated);

#endif

    // Camera devices:
    videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    connect(&m_devices, &QMediaDevices::videoInputsChanged, this, &MainWindow::updateCameras);
    const QRect *m_vsize = &ui->viewfinder->geometry();
    m_size = new QSize( m_vsize->width(),m_vsize->height());
    //    qDebug() << "TERJE:::::::" <<  m_vsize->height() << "    "  <<  m_vsize->width();
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

    m_captureSession.setVideoOutput(ui->viewfinder);
    ui->viewfinder->resize(*m_size);
    ui->viewfinder->show();

    /*Initialize a QImageCapture instance*/
    m_capture = new QImageCapture;
    m_captureSession.setImageCapture(m_capture);

    QDir dir(IMAGES_DIR);
    if (!dir.exists()){
        dir.mkpath(".");
    }

    /*Initialize a QImageCapture instance*/
    connect(m_capture, &QImageCapture::imageCaptured,[this](int id, const QImage &preview) {
        qDebug() << "Image Captured...";
        ui->plainTextEdit->appendPlainText(QString("Image Captured:%1").arg(id));
        QDateTime date = QDateTime::currentDateTime();
        QString filename2 = IMAGES_DIR+date.toString("yyyy_dd_MM_hh_mm_ss").append(".jpg");
        QPixmap pixmap(QPixmap::fromImage(preview));
        QImage *imagex = new QImage(pixmap.toImage());
        imagex->save(filename2, "jpg", 100);
    });

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
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString("flightlog.txt"));
    m_takeoffTime = QDateTime::currentDateTime();
    QString data = "Takeoff at: "+m_takeoffTime.toString();
    ui->listView->appendPlainText(data);

    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
        l_file->write(data.toLocal8Bit()+"\n");
        l_file->close();
    }
}

void MainWindow::logLanded()
{ 
    m_landedTime = QDateTime::currentDateTime();
    QString data = "Landed at: "+m_landedTime.toString();
    QTime dif(0,0,0,0);
    QTime t=dif.addMSecs(m_landedTime.toMSecsSinceEpoch()-m_takeoffTime.toMSecsSinceEpoch());
    QString result=t.toString("hh:mm:ss.zzz");
    QString dtime = "Duration: "+result;
    ui->listView->appendPlainText(data);
    ui->listView->appendPlainText(dtime);

    QFile *l_file = new QFile(QString(LOG_DIR)+ QString("flightlog.txt"));
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
        l_file->write(data.toLocal8Bit()+"\n");
        l_file->write(dtime.toLocal8Bit()+"\n");
        l_file->close();
    }
}

void MainWindow::doClock()
{
    static int tim = 0;

    if(!(++tim % 10)){
        static int lastPower = -1;
        if(lastPower != mysocket->Electricity){
            ui->plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString() + "IMU Power: "+  QString::number(mysocket->Electricity));
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
        // If more than 30Km/t we are taking off...
        double alt = m_preasure_alt;
        if(this->ui->radioButton_2->isChecked()){
            alt = this->m_altitude*3.2808399;
        }

        if( this->m_speed > 20.0 && alt > (m_alt + 5) ){
            m_takeoff = true;

            qDebug() << "takeoff...";

            this->takeoff_latitude  = this->m_latitude;
            this->takeoff_longitude = this->m_longitude;
            this->takeoff_altitude  = this->m_altitude;

            logTakeoff();
        }
    }

// Calculate meters/s ...
#define varfilterlength 4

    static double vario = 0;
    double var,ms;

    if(m_pressure_reader){
        var = this->m_preasure_alt - vario;
        vario = this->m_preasure_alt;
        var*=60;  // Feet/m...
    }
    else{
        ms = this->m_altitude - vario;
        vario = this->m_altitude;
        var=ms*3.2808399*60;  // Feet/m...
    }

    static double varfilter[varfilterlength]={0};
    double var_speed = 0;
    for(int x=0; x < varfilterlength-1;x++){
        varfilter[x] = varfilter[x+1];
        var_speed+=varfilter[x];
    }
    varfilter[varfilterlength-1] = var;
    var_speed+=var;
    m_var_speed=var_speed/varfilterlength;
}

void MainWindow::onGyroReadingChanged()
{
    m_gyro_reader = m_gyro_sensor->reading();
    mysocket->AsX  = m_gyro_reader->x();
    mysocket->AsY  = m_gyro_reader->y();
    mysocket->AsZ  = m_gyro_reader->z();
}

void MainWindow::onMagReadingChanged()
{
    m_mag_reader = m_mag_sensor->reading();
    mysocket->HX   = m_mag_reader->x()*2*M_PI;
    mysocket->HY   = m_mag_reader->y()*2*M_PI;
    mysocket->HZ   = m_mag_reader->z()*2*M_PI;
}

void MainWindow::onAccelerometerReadingChanged()
{
#define GFILTER 100
    static double avg=0;
    double tmp=0;

    m_accel_reader = m_accel_sensor->reading();
    mysocket->AccX = m_accel_reader->x();
    mysocket->AccY = -m_accel_reader->y();
    mysocket->AccZ = -m_accel_reader->z();
    tmp = sqrt(m_accel_reader->x()*m_accel_reader->x() +
               m_accel_reader->y()*m_accel_reader->y() +
               m_accel_reader->z()*m_accel_reader->z());
    avg -= avg/GFILTER;
    avg += tmp/GFILTER;

    if(m_calibrate > 0){
        if(m_calibrate == 1){
            m_first = 20;
            mysocket->G = avg;
            ekf.ekf->Gval = avg;
            ekf.Gval = avg;
        }
        m_calibrate--;
    }
}

void MainWindow::onCompassReadingChanged()
{
    m_compass_reader = m_compass_sensor->reading();
    m_heading = m_compass_reader->azimuth();
}

void MainWindow::onOrientationReadingChanged()
{
    m_orientation_reader = m_orientation_sensor->reading();
    mysocket->Orient  = m_orientation_reader->orientation();// x();  // when rotated...
}

void MainWindow::onRotationReadingChanged()
{
    m_rotation_reader = m_rotation_sensor->reading();
    mysocket->AngleX  = -m_rotation_reader->x();  // when rotated...
    mysocket->AngleY  = -m_rotation_reader->y()-90; // TERJE
    mysocket->AngleZ  = m_rotation_reader->z()+0.00001;
}

void MainWindow::onPressureReadingChanged()
{
    //    qDebug() << "  onPressureReadingChanged  ";
    static bool first = true;

    m_pressure_reader = m_pressure_sensor->reading();
    m_preasure = m_pressure_reader->pressure()/100.0;
    m_preasure = m_preasure + (1013.25 - ui->doubleSpinBox->text().toDouble());

    m_preasure_alt = 145366.45 * (1.0 - std::pow(m_preasure / 1013.25,0.190284));

    if( this->m_speed < 1.0){ first = true; }
    // Sett takeoff altitude...
    if(first){
        first = false;
        m_alt = m_preasure_alt;
        qDebug() << "set takeoff baroalt";

    }
}

void MainWindow::positionUpdated(QGeoPositionInfo geoPositionInfo)
{
    static bool first = true;

    if (geoPositionInfo.isValid())
    {
        m_geopos = true;
        //locationDataSource->stopUpdates();
        QGeoCoordinate geoCoordinate = geoPositionInfo.coordinate();
        this->m_latitude  = geoCoordinate.latitude();
        this->m_longitude = geoCoordinate.longitude();
        this->m_altitude  = geoCoordinate.altitude();

        double speed     = geoPositionInfo.attribute(QGeoPositionInfo::GroundSpeed);
        double bearing   = geoPositionInfo.attribute(QGeoPositionInfo::Direction);
        this->m_speed    = speed*3.6;
        double vel_D     = -geoPositionInfo.attribute(QGeoPositionInfo::VerticalSpeed);

        if(this->m_speed > 10.0 && !isnan(bearing)){
            m_bearing    = bearing;
        }
        else{
            m_bearing    = 999;
        }
        // Get velocity vector...
        if(!(isnan(speed) || isnan(bearing) || isnan(vel_D)))
        {
            this->m_vel_E  = sin(bearing*DEG_TO_RAD)*speed;
            this->m_vel_N  = cos(bearing*DEG_TO_RAD)*speed;
            this->m_vel_D  = -geoPositionInfo.attribute(QGeoPositionInfo::VerticalSpeed);
            this->m_vel_active = true;
        }
        else{
            this->m_vel_E=this->m_vel_N=this->m_vel_D=0.0;
            this->m_vel_active = false;
        }

        //    this->m_head      = geoPositionInfo.attribute(QGeoPositionInfo::Direction);

        //        this->m_head_dir = geoCoordinate.azimuthTo(QGeoCoordinate(0,0));

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
        // set ground altitude...
        if(isnan(this->m_altitude)){
            this->m_altitude = 0;
        }else{
            // Set takeoff altitude...
            if(first && this->ui->radioButton_2->isChecked())
            {
                first = false;
                m_alt = this->m_altitude*3.2808399;
                // qDebug() << "set takeoff alt";
            }
            // As long as we do not move, but got altitude...
            if(speed < 0.5){
                first = true;
                // qDebug() << "reset takeoff alt";
            }
        }
    }
}

void MainWindow::EKF()
{
//    qDebug() << mysocket->AngleX << mysocket->AngleY << mysocket->AngleZ;

    if((m_gyro_reader != nullptr && m_accel_reader != nullptr) || (ekf.m_use_gpt == true))
    {
        static steady_clock::time_point clock_begin = steady_clock::now();
        steady_clock::time_point clock_end = steady_clock::now();
        steady_clock::duration time_span = clock_end - clock_begin;
        m_dt = double(time_span.count()) / 1000000000L;
        clock_begin=clock_end;

        // How to caulcate Euler Angles [Roll Φ(Phi) Gyro Z, Pitch θ(Theta) gyro Y, Yaw Ψ(Psi) Gyro X]
        if(ekf.m_use_gpt == false)
        {
            if(m_mag_reader == nullptr){
                mysocket->HX=mysocket->HY=mysocket->HZ=0;
            }
            Vector3x gyro = {mysocket->AsZ,mysocket->AsY,mysocket->AsX};
            Vector3x accel= {-mysocket->AccY,-mysocket->AccZ,mysocket->AccX};
            Vector3x mag  = {mysocket->HX,mysocket->HY,mysocket->HZ};

            // Rotate the roll axis with respect to pitch...
            double y = (gyro[2] * sin(ekf.getPitch_rad())) + (gyro[0] * cos(ekf.getPitch_rad())) ;
            double z = (gyro[2] * cos(ekf.getPitch_rad())) + (gyro[0] * sin(ekf.getPitch_rad())) ;

            //            double ay = (accel[0]* DEG_TO_RAD * sin(ekf.getPitch_rad())) + (accel[2]* DEG_TO_RAD * cos(ekf.getPitch_rad())) ;
            //          double az = (accel[0]* DEG_TO_RAD * cos(ekf.getPitch_rad())) + (accel[2]* DEG_TO_RAD * sin(ekf.getPitch_rad())) ;

            // Estimate using accelerometers...
            std::tie(m_pitch,m_roll,m_yaw) = ekf.getPitchRollYaw(accel[1],accel[0],accel[2],mag[0],mag[1],mag[2],ekf.Gval);
            if(isnan(m_pitch)) m_pitch = 0;
            if(isnan(m_roll)) m_roll = 0;
            if(isnan(m_yaw)) m_yaw = 0;

            // Calculates X and Y relative distances in meters.
            double ypos = 0;
            double xpos = 0;
            if(!(isnan(this->m_latitude) || isnan(this->m_longitude))){
                double deltaLatitude = this->m_latitude - this->takeoff_latitude;
                double deltaLongitude = this->m_longitude - this->takeoff_longitude;
                double latitudeCircumference = 40075160 * cos(this->takeoff_latitude*DEG_TO_RAD);
                ypos = deltaLongitude * latitudeCircumference / 360;
                xpos = deltaLatitude * 40008000 / 360;
            }

            // get speed vector...
            double vel_N = 0.0;
            double vel_E = 0.0;
            double vel_D = 0.0;
            if (this->m_vel_active == true){
                vel_N = this->m_vel_N;
                vel_E = this->m_vel_E;
                vel_D = this->m_vel_D;
            }

            // update kalmanfilter
            ekf.ekf_update( m_dt,
                           vel_N,
                           vel_E,
                           vel_D,
                           xpos, //this->m_latitude, //xpos, // * 1e-7,
                           ypos, //this->m_longitude, //ypos, // * 1e-7,
                           -m_altitude, // * 1e-3,
                           -m_preasure_alt*0.3048, // Meter NED...
                           -gyro[1]* DEG_TO_RAD,
                           y* DEG_TO_RAD, //gyro[0] * DEG_TO_RAD,
                           z* DEG_TO_RAD, // gyro[2] * DEG_TO_RAD,
                           m_pitch,
                           m_roll,
                           m_yaw,
                           mag[0], // Z
                           mag[1], // Y
                           mag[2]  // X
                           );

#define SCALE 1.00
            double roll = (ekf.getRoll_rad() * RAD_TO_DEG) + (SCALE * z * m_dt);
            m_attitude = {roll,ekf.getPitch_rad()* RAD_TO_DEG,ekf.getHeading_rad()* RAD_TO_DEG};
        }
        else{
            m_attitude =  {mysocket->AngleX,mysocket->AngleY,mysocket->AngleZ};
            m_heading = mysocket->AngleZ;
        }
    }
}

#include <chrono>

void MainWindow::onReadingChanged()
{
#define filterlength 30
    static double filter[filterlength]={0};
#define rotfilterlength 20
    static double rotfilter[rotfilterlength]={0};
#define headfilterlength 3
    static double headfilter[headfilterlength]={0};
    double x_head = 0.0;
    double slipp = 0.0;
    static int calib_required = 0;
    static bool running = false;

 //   if(calib_required > 0 && mysocket->AngleX != 0.0 && mysocket->AngleY != 0.0 && mysocket->AngleZ != 0.0)
 //   {
 //       calib_required--;
 //   }

    // Sensor rotation...
    if(calib_required == 0)
    {
        Vector3x gyro = {mysocket->AsZ,mysocket->AsY,mysocket->AsX};
        Vector3x accel= {-mysocket->AccY,-mysocket->AccZ,mysocket->AccX};
        Vector3x attitude  = m_attitude;

        if(m_first > 0){
            m_first--;

            if(m_first == 1){
                if(!this->m_msgBox->isHidden()){
                    this->m_msgBox->hide();
                }

                if(!(this->m_msgBoxCalibrating == nullptr))
                {
                    this->m_msgBoxCalibrating->hide();
                    delete(this->m_msgBoxCalibrating);
                    this->m_msgBoxCalibrating = nullptr;
                }

                m_offset = attitude[1];
                m_acc_Y_calib = accel[0];
            }
        }


        if(m_bearing != 999){
            x_head = m_bearing;
        }
        else{
            x_head = m_heading - heading_offset;
        }
        slipp = (accel[0]-m_acc_Y_calib)*9.5;

        if(mysocket->Transponderstat == "false" &&  ui->radioButton->isCheckable()){
            ui->radioButton->setChecked(false);
            ui->radioButton->setCheckable(true);
            ui->radioButton_3->setChecked(false);
        }
        else if(mysocket->Transponderstat == "true" &&  !ui->radioButton->isCheckable()){
            ui->radioButton->setCheckable(true);
        }

        // Make compass...
        double head_dir = 0;
        for(int x=0; x < headfilterlength-1;x++){
            headfilter[x] = headfilter[x+1];
            head_dir+=headfilter[x];
        }
        headfilter[headfilterlength-1] = x_head;
        head_dir+=x_head;
        head_dir/=headfilterlength;

        m_head = head_dir;

        // Store data in local registers...
        double roll_att = 0;
        double pitch_att = 0;

        roll_att  = -attitude[0];
        pitch_att = 1 * (attitude[1] - m_offset);

        double r = gyro[2];//*2;//((sin(attitude[2]*DEG_TO_RAD)*gyro[0])+(cos(attitude[2]*DEG_TO_RAD)*gyro[2]));
        if(r >  6.0)r =  6.0;
        if(r < -6.0)r = -6.0;

        double rot_speed = 0;
        for(int x=0; x < rotfilterlength-1;x++){
            rotfilter[x] = rotfilter[x+1];
            rot_speed+=rotfilter[x];
        }
        rotfilter[rotfilterlength-1] = r;
        rot_speed+=r;
        rot_speed/=-rotfilterlength;

        // Make the turn bank ...
        double slippIndicator = 0;
        if(slipp >  16.0)slipp =  16.0;
        if(slipp < -16.0)slipp = -16.0;
        for(int x=0; x < filterlength-1;x++){
            filter[x] = filter[x+1];
            slippIndicator+=filter[x];
        }
        filter[filterlength-1] = slipp;
        slippIndicator+=filter[filterlength-1];
        slippIndicator/=filterlength;


        if( currentIndex == 2)
        {
            static int xtimer=0;

            if(++xtimer%3 == 0){
                if( ui->radioButton_3->isChecked()){
                    _widgetALT->setAltitude(m_preasure_alt);
                }else if( ui->radioButton_2->isChecked()){
                    _widgetALT->setAltitude(this->m_altitude*3.2808399);
                }else{
                    _widgetALT->setAltitude(m_tansALT);
                }
                _widgetALT->redraw();

                if(this->m_speed >= 0 && this->m_speed < 300)
                    _widgetASI->setAirspeed(this->m_speed);
                else
                    _widgetASI->setAirspeed(0);

                _widgetASI->redraw();

                _widgetHI->setHeading(-m_head );
                _widgetHI->redraw();
            }

            _widgetAI->setRoll ( roll_att );
            _widgetAI->setPitch( pitch_att );
            _widgetAI->redraw();

            if(xtimer%5 == 0){
                _widgetTC->setTurnRate(rot_speed);
                _widgetTC->setSlipSkid(slippIndicator);
                _widgetTC->redraw();

                _widgetVSI->setClimbRate(m_var_speed);
                _widgetVSI->redraw();
            }
        }

        if( currentIndex == 3)
        {
            if( ui->radioButton_3->isChecked()){
                _widgetEADI->setAltitude( m_preasure_alt );
            }else if( ui->radioButton_2->isChecked()){
                _widgetEADI->setAltitude(this->m_altitude*3.2808399 );
            }else{
                _widgetEADI->setAltitude( m_tansALT);
            }
            _widgetEADI->setClimbRate(m_var_speed);
            _widgetEADI->setPressure( m_preasure, qfi_EADI::PressureMode::MB );

            _widgetEADI->setHeading( m_head );
            _widgetEADI->setSlipSkid( slippIndicator/57 );
            _widgetEADI->setTurnRate( rot_speed );
            _widgetEADI->setRoll( roll_att );
            _widgetEADI->setPitch(pitch_att );

            _widgetEADI->setFltMode( qfi_EADI::FltMode::CMD);
            _widgetEADI->setSpdMode( qfi_EADI::SpdMode::FMC_SPD);
            _widgetEADI->setLNAV( qfi_EADI::LNAV::APR_ARM);
            _widgetEADI->setVNAV( qfi_EADI::VNAV::ALT);
            _widgetEADI->setFPM( 0.0, 0.0 );
            _widgetEADI->setDots( 0.0,0.0,false,false);
            _widgetEADI->setFD( 0.0,0.0,true);
            _widgetEADI->setStall( false );
            _widgetEADI->setMachNo( 0.0 );
            _widgetEADI->setAirspeedSel( 80.0 );
            _widgetEADI->setAltitudeSel( 400 );
            _widgetEADI->setHeadingSel( 80.0 );
            _widgetEADI->setVfe( 120.0 );
            _widgetEADI->setVne( 160.0 );
            _widgetEADI->redraw();

            _widgetEHSI->setHeading( 0 );
            _widgetEHSI->setCourse( m_head );
            _widgetEHSI->setBearing( m_head, true );
            _widgetEHSI->setDeviation( 0.0, qfi_EHSI::CDI::FROM);
            _widgetEHSI->setDistance( 200, true );
            _widgetEHSI->setHeadingSel( m_head );
            _widgetEHSI->redraw();
        }

        if( currentIndex == 1)
        {
            QGraphicsScene * m_graphScen = new QGraphicsScene;
            QSize x = ui->graphicsView->size();

            m_graphScen->setSceneRect(0,0,x.width(),x.height());
            float x1 = cos(-roll_att/(180.0/3.1415));
            float y1 = sin(-roll_att/(180.0/3.1415));
            float z1 = sin(pitch_att/(180.0/3.1415));

            int offset = pitch_att;
            if (offset > 40) offset = 40;
            if (offset < -40) offset = -40;
            offset = offset * (x.height()/180.0);

            // -----------------------------------------
            // Artificial Horizon line...
            m_graphScen->addLine((int)((x.width() /2)-(x1*200)),
                                 (int)((x.height()/2)-(y1*200))+offset,
                                 (int)((x.width() /2)+(x1*200)),
                                 (int)((x.height()/2)+(y1*200))+offset,
                                 QPen(QBrush(Qt::yellow),6));

            if( roll_att > 60 || roll_att < -60 || pitch_att > 30 || pitch_att < -30)
            {
                ui->graphicsView->setBackgroundBrush(QBrush(Qt::red));
            }
            else
            {
                ui->graphicsView->setBackgroundBrush(QBrush(Qt::black));
            }

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

            m_graphScen->addLine((int)175,
                                 (int)((x.height()/1)-100),
                                 (int)x.width()-175,
                                 (int)((x.height()/1)-100),
                                 QPen(QBrush(Qt::white),1));

            // Horisontal bottom line...
            m_graphScen->addEllipse((slippIndicator*4)+((x.width()/2)-11),x.height()-110,20,20,QPen(Qt::white),QBrush(Qt::cyan));

            // -----------------------------------------
            // Center main angle...
            m_graphScen->addLine((int)((x.width() /2)-(x1*60)),
                                 (int)((x.height()/2)-(y1*60)),
                                 (int)((x.width() /2)+(x1*60)),
                                 (int)((x.height()/2)+(y1*60)),
                                 QPen(QBrush(Qt::white),2,Qt::PenStyle(Qt::DashLine)));

            // Center main angle...
            float p_loc_A = (x.width() /2) + ((int)m_head % 60);
            float p_loc_B = (x.height() /2);

            m_graphScen->addLine((int)(p_loc_A),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            m_graphScen->addLine((int)(p_loc_A-60),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A-60),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            m_graphScen->addLine((int)(p_loc_A+60),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A+60),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            m_graphScen->addLine((int)(p_loc_A-120),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A-120),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            m_graphScen->addLine((int)(p_loc_A+120),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A+120),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            m_graphScen->addLine((int)(p_loc_A-180),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A-180),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            m_graphScen->addLine((int)(p_loc_A+180),
                                 (int)(p_loc_B-10),
                                 (int)(p_loc_A+180),
                                 (int)(p_loc_B+10),
                                 QPen(QBrush(Qt::white),3,Qt::PenStyle(Qt::SolidLine)));

            // First upper bar...
            p_loc_A = (x.width() /2)  + (y1*15.0);
            p_loc_B = (x.height() /2) - (x1*15.0);

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

#ifndef Q_OS_MAC
            if(mysocket->IMUconnected == true)
            {
                ui->roll->setText(QString("%1").arg(abs(roll_att), 0, 'f', 0));
                ui->pitch->setText(QString("%1").arg((pitch_att), 0, 'f', 0));
                ui->temp->setText(QString("%1").arg(mysocket->Electricity, 0, 'f', 0));
                ui->temperature->setText(QString("%1").arg(mysocket->Temperature, 0, 'f', 1));
                ui->compass->setText(QString("%1").arg(m_head, 0, 'f', 0));
            }
            else if(m_rotation_sensor != nullptr)
#endif
            {
                ui->roll->setText(QString("%1").arg(abs(roll_att), 0, 'f', 0));
                ui->pitch->setText(QString("%1").arg((pitch_att), 0, 'f', 0));
                ui->compass->setText(QString("%1").arg(m_head, 0, 'f', 0));
            }

            if( m_geopos == true){
                ui->altitude->setText(QString("%1").arg(m_altitude*3.2808399, 0, 'f', 0));
            }
        }
        m_reading|=0x04;
    }
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
    QString x = ui->pushButton_11->styleSheet();
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

void MainWindow::getVal(void *parent, QByteArray array)
{
    static char buffer[30];
    static int pos = 0;
    MainWindow* saved_this= (MainWindow*) parent;
    Ui::SCREEN* local_ui  = saved_this->ui;

    for(int i=0; i < array.size();i++)
    {
        if(array.at(i) == '*')
        {
            QString x;
            x = local_ui->pushButton_10->styleSheet();
            x.replace(QString("1 #900"), QString("1 #090"));
            local_ui->pushButton_10->setStyleSheet(x);
            local_ui->pushButton_10->update();
            saved_this->timerPing->start(10000); // Turn off in 10 sec...
        }

        if(array.at(i) < 0x1F || pos >= (int)sizeof(buffer))
        {
            if(pos >= 3)
            {
                // Log all commands... This might be slow... will look at a timed write...
                QFile *l_file = new QFile(QString(LOG_DIR)+ QString("log.txt"));
                if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
                {
                    QString data = QDateTime::currentDateTime().toString()+": "+buffer;
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

                switch (buffer[0])
                {
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
                        sscanf(buffer,"a=%f",&number);
                        QString altType="Alt.Ft.";

                        if(saved_this->alt_mode == 1)
                        {
                            for (unsigned long key=0; key < strlen(buffer); key++)
                                if(buffer[key]=='M')
                                {
                                    number*=3.2808399;
                                    break;
                                }
                            saved_this->m_tansALT = round(number/100.0)*100;
                        }
                        else{
                            for (unsigned long key=0; key < strlen(buffer); key++)
                                if(buffer[key]=='F')
                                {
                                    number/=3.2808399;
                                    break;
                                }
                            saved_this->m_tansALT = round(number);
                            altType="Alt.M.";
                        }
                        snprintf(numout,20,"%.4d",(int)saved_this->m_tansALT);
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
            if(array.at(i) >= 0x1F)
            {
                buffer[pos++]=array.at(i);
                buffer[pos]=0;
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

}

void MainWindow::on_pushButton_18_clicked()
{
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

// Is 1 offset 0 Transponder
void MainWindow::on_select_camera_from_transponder_clicked() {
    setCamera(QMediaDevices::defaultVideoInput());
    ui->stackedWidget->setCurrentIndex(6);
    currentIndex = 6;
}
void MainWindow::on_select_gyro_page_clicked()          {
    ui->stackedWidget->setCurrentIndex(1);
    currentIndex = 1;
}

// Is 2 offset 1 Gyro eng
void MainWindow::on_select_transponder_page_clicked()   {
    if( mysocket->Transponderstat == "true")            {
        ui->stackedWidget->setCurrentIndex(0);
        currentIndex = 0;
    }
    else{
        setCamera(QMediaDevices::defaultVideoInput());
        ui->stackedWidget->setCurrentIndex(6);
        currentIndex = 6;
    }
}
void MainWindow::on_select_dumy_page2_clicked()         {
    ui->stackedWidget->setCurrentIndex(2);
    currentIndex = 2;
}

// Is 3 offset 2 gyro nice
void MainWindow::on_select_transponder_page_2_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
    currentIndex = 1;
}
void MainWindow::on_select_dumy_page2_2_clicked()       {
    ui->stackedWidget->setCurrentIndex(3);
    currentIndex = 3;
}

// Is 4 offset 3 eadi
void MainWindow::on_select_transponder_page_3_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    currentIndex = 2;
}
void MainWindow::on_select_from_4_to_5_clicked()        {
    ui->stackedWidget->setCurrentIndex(5);
    currentIndex = 5;
}

// Is 5 offset 4 map
void MainWindow::on_select_gyro_page2_2_clicked()       {
    ui->stackedWidget->setCurrentIndex(3);
    currentIndex = 3;
}
void MainWindow::on_select_transponder_page2_2_clicked(){
    ui->stackedWidget->setCurrentIndex(4);
    currentIndex = 4;
}

// Is 6 offset 5 users
void MainWindow::on_select_gyro_page2_clicked()         {
    ui->stackedWidget->setCurrentIndex(5);
    currentIndex = 5;
}
void MainWindow::on_select_transponder_page2_clicked()  {
    setCamera(QMediaDevices::defaultVideoInput());
    ui->stackedWidget->setCurrentIndex(6);
    currentIndex = 6;
}

// Is 7 offset 6  camera
void MainWindow::on_select_transponder_page2_3_clicked(){
    if( mysocket->Transponderstat == "true"){
        hideCamera();
        ui->stackedWidget->setCurrentIndex(0);
        currentIndex = 0;
    }
    else{
        hideCamera();
        ui->stackedWidget->setCurrentIndex(1);
        currentIndex = 1;
    }
}

void MainWindow::on_select_gyro_page2_3_clicked(){
    hideCamera();
    ui->stackedWidget->setCurrentIndex(4);
    currentIndex = 4;
}

//-------------------------------------

void MainWindow::on_imu_reset_clicked()
{
    m_calibrate = 500;
    m_msgBoxCalibrating = new QMessageBox(QMessageBox::Information,tr("Full Calibrating!"),tr("Please make sure the equipment is still and wait!"));
    m_msgBoxCalibrating->setStandardButtons(QMessageBox::NoButton);
    m_msgBoxCalibrating->show();
}

void MainWindow::on_use_hw_clicked()
{
    m_first = 10;
    m_msgBoxCalibrating = new QMessageBox(QMessageBox::Information,tr("Fast Calibrating!"),tr("Please make sure the equipment is still and wait!"));
    m_msgBoxCalibrating->setStandardButtons(QMessageBox::NoButton);
    m_msgBoxCalibrating->show();
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
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(RADIO));
    if( l_file->open(QIODevice::ReadWrite )){
        QString data = ui->textEdit->toPlainText();
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }
}


void MainWindow::on_textEdit_2_textChanged()
{
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(AIRPLANE));
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


    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(RADIO));
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
    ui->textEdit_2->insertPlainText("LN-YRI\tPeregrin__\tRingerike\n");
    ui->textEdit_2->insertPlainText("LN-YRM\tAeroprak\tRingerike\n");
    ui->textEdit_2->insertPlainText("LN-YZG\tIcarus\t\tMorten\n");
    ui->textEdit_2->insertPlainText("LN-YYV\tIcarus\t\tTerje S.\n");
    ui->textEdit_2->insertPlainText("LN-YPL\tIcarus\t\tOlaf\n");
    ui->textEdit_2->insertPlainText("LN-YOG\tSamba\t\tJan Ove\n");
    ui->textEdit_2->insertPlainText("LN-YRY\tRans S6S\tAlf Nipe\n");

    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(AIRPLANE));
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

void MainWindow::on_reconnect_now_clicked()
{
    delete(mysocket);
    mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal, &this->setIMU);
}

void MainWindow::on_pushButton_20_clicked()
{
    qDebug() << "--------Take picture --------- " ;   
    m_capture->capture();
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

void MainWindow::on_reset_heading_clicked()
{
    heading_offset = mysocket->AngleZ;
    //    heading_offset = 180.0 - mysocket->AngleZ;
    //    if(heading_offset<0.0) heading_offset+=360.0;

    if(m_compass_sensor){
        //      m_compass_reader->setAzimuth(heading_offset);
    }
}

void MainWindow::on_reset_att_clicked()
{
    qDebug() << "Use IMU clicked... ";
    if(ekf.m_use_gpt == true){
        ekf.m_use_gpt = false;
        QString x = ui->reset_att->styleSheet();
        x.replace(QString("1 #f00"), QString("1 #888"));
        ui->reset_att->setStyleSheet(x);
        ui->reset_att->update();
    }
    else{
        ekf.m_use_gpt = true;
        QString x = ui->reset_att->styleSheet();
        x.replace(QString("1 #888"), QString("1 #f00"));
        ui->reset_att->setStyleSheet(x);
        ui->reset_att->update();
    }
}

void MainWindow::on_fly_home_clicked()
{
    qDebug() << "FlyHome... ";
}

void MainWindow::on_dial_valueChanged(int qnh)
{
    ui->doubleSpinBox->setText(QString("%1").arg(qnh/100.0));
    ui->doubleSpinBox_2->setText(QString("%1").arg(qnh/100.0));
    ui->dial_2->setSliderPosition(qnh);
}

void MainWindow::on_dial_2_valueChanged(int qnh)
{
    ui->doubleSpinBox_2->setText(QString("%1").arg(qnh/100.0));
    ui->doubleSpinBox->setText(QString("%1").arg(qnh/100.0));
    ui->dial->setSliderPosition(qnh);
}
