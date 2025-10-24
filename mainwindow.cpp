//
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
#include <QPermission>
#include <QActionGroup>
#include <QVideoWidget>
#include <QCameraDevice>
#include <QPixmap>
#include <QMediaRecorder>
#include <QImageCapture>
#include <QMediaFormat>
#include <QMediaPlayer>
#include <QOrientationSensor>
#include <QImageCapture>
#include <QList>
#include <QSplashScreen>
#include <QtMath>  // for qDegreesToRadians, qRadiansToDegrees
#include <deque>
#include <QThread>

//***C++11 Style:***
#include <chrono>

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

#include "mainwindow.h"
//#include "mytcpsocket.h"
#include "gpx_parse.h"

#include "wit_c_sdk.h"

using namespace std;
using namespace std::chrono;

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

#ifdef Q_OS_IOS
    // Documents directory (user-visible, backed up)
    QString documentsPath = LOG_DIR;
    QDir dir(documentsPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
#endif

    ui->setupUi(this);

#ifdef Q_OS_IOS
    QPixmap splashPixmap(":/splash.png");  // Or use a file path
    splash = new QSplashScreen(splashPixmap);
    splash->autoFillBackground();
//    splash->showMessage("Initializing Flight IMU...", Qt::AlignTop | Qt::AlignCenter, Qt::black);
    splash->show();
#endif

#if defined(Q_OS_ANDROID) && defined(USE_KeepAwakeHelper)
    helper = new KeepAwakeHelper();
    helper->EnableKeepAwakeHelper();
#endif

    // --------------------------------
    // Setup the Radio List...
    double temp_g=0.0;
    qDebug() << "Reading files...";
    QString blob1;
    {
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString(RADIO));
        if (l_file->open(QIODevice::ReadOnly))
        {
            blob1 = l_file->readAll();
            l_file->close();
        }
        else{
            set_default_radio();
        }
    }
    // Setup the Airplane List...
    QString blob2;
    {
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString(AIRPLANE));
        if (l_file->open(QIODevice::ReadOnly))
        {
            blob2 = l_file->readAll();
            l_file->close();
        }
        else{
            set_default_planes();
        }
    }
    {
        // Log all commands... This might be slow... will look at a timed write...
        QFile *l_file = new QFile(QString(LOG_DIR)+QString(TRANSPONDERLOG));
        if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
        {
            QString data = QDateTime::currentDateTime().toString()+": New Log: \n";
            l_file->write(data.toLocal8Bit());
            l_file->close();
        }
    }

    ui->textEdit->insertPlainText(blob1);
    ui->textEdit_2->insertPlainText(blob2);

    // --------------------------

   // ui->quickWidget->setSource(QUrl("qrc:/places_map.qml"));
   // ui->quickWidget->rootObject()->setProperty("zoomLevel", 35); // 18);

    ui->lcdNumber->display(QString::number(this->current[0]*1000+this->current[1]*100+this->current[2]*10+this->current[3]).rightJustified(4, '0'));
    ui->lcdNumber_2->display(QString::number(this->next[0]*1000+this->next[1]*100+this->next[2]*10+this->next[3]).rightJustified(4, '0'));
    ui->plainTextEdit->appendPlainText("Glasscockpit 200-UAV v1.02a");
    ui->listView->appendPlainText("Glasscockpit 200-UAV v1.02a");
    ui->listView->raise();
    ui->listView->setHidden(true);

    // This code must be rewritten as it is depending on timeing and speed.
    // The serial ports shuld be a parameter to the constructor...
    // The m_calibrate shuld be set in a different manner...
    // Remember that the MyTcpSocket spawns a slower process only...
    mysocket = new MyTcpSocket(this, ui->plainTextEdit, &this->getVal, &this->setIMU);
    mysocket->setSerialPorts(_IMU_id, _transponder_id, _radar_id); // This must come fast to make sure it is set before the serialports are open.
    QThread::msleep(1000);

    // Setup the Config List...
    Matrix3x6 config;
    memset(&config,0,sizeof(Matrix3x6));
    if(get_default_config(config) != 0)
    {
        config[0][2] = (double)Gfix;
        set_default_config(config);
    }

    qDebug() << "AccValue" << config[0][2];
    temp_g = config[0][2];
    ekf.ekf->Gval       = temp_g;
    ekf.ekf_quart->Gval = temp_g;
    ekf.Gval            = temp_g;
    mysocket->G   = temp_g;
    qDebug() << "Saved G value is: " << temp_g;

    // Whenever the location data source signals that the current
    // position is updated, the positionUpdated function is called.
    this->m_geoPositionInfo = QGeoPositionInfoSource::createDefaultSource(this);
    if (this->m_geoPositionInfo)
    {
        connect(m_geoPositionInfo,SIGNAL(positionUpdated(QGeoPositionInfo)),this,SLOT(positionUpdated(QGeoPositionInfo)));
        // Start listening for position updates
        m_geoPositionInfo->setUpdateInterval(1000);
        m_geoPositionInfo->startUpdates();
    }
    // ------------------------------

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
    timerPing->setSingleShot(true);
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

    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(FLIGHTLOG));
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
    {
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

    //---------------------------------------------------------------

//#if defined(Q_OS_ANDROID) || defined(Q_OS_MAC)

    // Open for MQTT Input from simulator and external senors... Only work on OSX for now...
    SERVER_ADDRESS = std::string("tcp://localhost:1883");
    CLIENT_ID = std::string("transponder");

    // MQTT setup
    mqtt = new MqttClient(SERVER_ADDRESS, CLIENT_ID);
    mqtt->setMessageHandler([this](const std::string& topic, const std::string& payload)
        {
            try {
                float value = std::stof(payload);
                this->handleUpdate(topic, value);
            } catch (const std::exception& e) {
                qWarning() << "Invalid float in payload:" << QString::fromStdString(payload)
                << "Error:" << e.what();
            }
        });

    mqtt->connect();
    mqtt->subscribe("xplane/#");
    try {
        mqtt->sendMessage("xplane/topic",  "1.0 eMove GUI Controller!");
    } catch (const mqtt::exception& e) {
        printf("MQTT publish error: %s\n", e.what());
    }
//#endif
    // -----------------------------------------------------------

    m_msgBox = new NoButtonMessageBox(tr("Please wait for the system to boot!"));
//    m_msgBox->show();

    showImage();

//    QGraphicsItem *item = ui->graphicsView_3->scene()->items().first();
//    ui->graphicsView_3->fitInView(item, Qt::KeepAspectRatio);

    qDebug() << "  Booted...  " << currentIndex << " Index";

    // Quantum computer test, just to see if it would work...
//    qDebug() << "  Running Quantum-computing problem to test device speed...  ";
//    Qiskit();

    // Show splash until done
    /*
//    while(m_bluetoothrunning == true){
    for( int i = 0; i < 5; i++){
        QCoreApplication::processEvents();
        QThread::msleep(1000);
    }
*/
}

MainWindow::~MainWindow()
{
    qDebug() << "Exiting...";
    qApp->closeAllWindows();
}


void MainWindow::showImage()
{
    auto *view = ui->graphicsView_3;                 // <-- use one view consistently
    QGraphicsScene *scene = view->scene();
    if (!scene) {
        scene = new QGraphicsScene(view);
        view->setScene(scene);
    }

    QPixmap pix(":/test1.png");                      // from resources
    if (pix.isNull()) {
        qWarning() << "Failed to load :/test1.png; check .qrc path";
        return;
    }

    scene->clear();
    QGraphicsPixmapItem *item = scene->addPixmap(pix);
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());

    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // Fit now; if the view isn't laid out yet, refit on next event loop tick
    view->fitInView(item, Qt::IgnoreAspectRatio);
    QTimer::singleShot(0, view, [view, item]{
        view->fitInView(item, Qt::IgnoreAspectRatio);
    });

    ui->fly_home->setText("");
    ui->fly_home->setIcon(QIcon(":/engage.png"));
    ui->fly_home->setFlat(true); // optional
    QSize f = ui->fly_home->size();
    ui->fly_home->setIconSize(f);

    /*

QPushButton {
image: url(:/backwards.svg);
color: #333;
border: 2px solid #555;
border-radius: 20px;
border-style: outset;
background: qradialgradient(
cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,
radius: 1.35, stop: 0 #fff, stop: 1 #15E
);
padding: 5px;
}

QPushButton:pressed {
    background-color: rgb(224, 0, 0);
    border-style: inset;
}




QPushButton {
color: #333;
border: 2px solid #555;
border-radius: 20px;
border-style: outset;
background: qradialgradient(
cx: 0.3, cy: -0.4, fx: 0.3, fy: -0.4,
radius: 1.35, stop: 0 #fff, stop: 1 #068
);
padding: 5px;
}

QPushButton:pressed {
    background-color: rgb(224, 0, 0);
    border-style: inset;
}

*/



}
//***************************************************************************************************************//
/**
 * @brief Read in data from X-Plane and send to IQEngine...
 *
 * @param in
 */

//#if defined(Q_OS_ANDROID) || defined(Q_OS_MAC)

void MainWindow::handleUpdate(const std::string& ID, float value)
{
    static bool first = true;
    static int statusCount = 0;

    qDebug() << "[MQTT] " << QString::fromStdString(ID) << " = " << value;

    if (ID == "xplane/topic")
    {
        qDebug() << "TOPIC: " << QString::fromStdString(ID);
    }
    else if (ID == "xplane/ax")
    {
        m_has_MQTT_gyro = true;
        mysocket->AsX = value;
        statusCount++;
    }
    else if (ID == "xplane/ay")
    {
        m_has_MQTT_gyro = true;
        mysocket->AsY = value;
        statusCount++;
    }
    else if (ID == "xplane/az")
    {
        m_has_MQTT_gyro = true;
        mysocket->AsZ = value;
        statusCount++;
    }
    else if (ID == "xplane/rollRate")
    {
        m_has_MQTT_accel = true;
        statusCount++;
    }
    else if (ID == "xplane/pitchRate")
    {
        m_has_MQTT_accel = true;
        statusCount++;
    }
    else if (ID == "xplane/yawRate")
    {
        m_has_MQTT_accel = true;
        statusCount++;
    }
    else if (ID == "xplane/climbRate")
    {
        m_has_MQTT_vsi = true;
        statusCount++;
    }
    else if (ID == "xplane/heading")
    {
        m_has_MQTT_heading = true;
        m_heading = value;
        statusCount++;
    }
    else if (ID == "xplane/airspeed")
    {      // AIR speed...
        m_has_MQTT_airspeed = true;
        m_gpsspeed = value;
        statusCount++;
    }
    else if (ID == "xplane/localPressure")
    {
        m_has_MQTT_preassure = true;
        statusCount++;
    }


    // Show ready status once key values received
    if (statusCount >= 4 && first)
    {
        first = false;
        m_has_MQTT = true;
    }
}
//#endif

//***************************************************************************************************************//
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
    if(use_imu) qDebug() << "Found a sensors...";
    else qDebug() << "NOT Found any sensors...";

    for (const QByteArray &type : QSensor::sensorTypes())
    {
        qDebug() << "Found a sensor type:" << type;

        for (const QByteArray &identifier : QSensor::sensorsForType(type))
        {
            qDebug() << "    " << "Found a sensor of that type:" << identifier;
            QSensor* sensor = new QSensor(type, local);
            sensor->setIdentifier(identifier);
            mySensorList.append(sensor);
        }

        if(!strncmp(type,"QPressureSensor",strlen("QPressureSensor")))
        {
            local->m_pressure_sensor = new QPressureSensor();
            connect(local->m_pressure_sensor, SIGNAL(readingChanged()), local, SLOT(onPressureReadingChanged()));
            local->m_pressure_sensor->start();
            local->m_pressure_sensor->setDataRate(4);
            qDebug() << "Found a sensor QPressureSensor";
            local->ui->radioButton_3->setChecked(true);

            // If we got both a preassure sensor and the Transponder ...
            if( local->mysocket->Transponderstat == true)
            {
                local->mysocket->readyWrite((char*)"d=s\r\n");
                local->mysocket->TransponderstatWithBarometer = true;

                QString x = local->ui->use_built_inn_barometer->styleSheet();
                x.replace(QString("1 #080"), QString("1 #800"));
                local->ui->use_built_inn_barometer->setText("Use Built In\nbarometer");
                local->ui->use_built_inn_barometer->setStyleSheet(x);
                local->ui->use_built_inn_barometer->update();
            }
        }

        if(!strncmp(type,"QOrientationSensor",strlen("QOrientationSensor")))
        {
            local->m_orientation_sensor = new QOrientationSensor();
            local->m_orientation_sensor->setDataRate(1);
            connect(local->m_orientation_sensor, SIGNAL(readingChanged()), local, SLOT(onOrientationReadingChanged()));
            local->m_orientation_sensor->start();
            qDebug() << "Found a sensor QRotationSensor";
        }

        if(!strncmp(type,"QAmbientTemperatureSensor",strlen("QAmbientTemperatureSensor")))
        {
            local->m_temp_sensor = new QAmbientTemperatureSensor();
            local->m_temp_sensor->setDataRate(1);
            connect(local->m_temp_sensor, SIGNAL(readingChanged()), local, SLOT(onTempReadingChanged()));
            local->m_temp_sensor->start();
            qDebug() << "Found a sensor QAmbientTemperatureReading";
        }

        // If we do not have an external IMU...
        if(local->m_use_imu  == false)
        {
            if(!strncmp(type,"QRotationSensor",strlen("QRotationSensor")))
            {
                local->m_rotation_sensor = new QRotationSensor();
                local->m_rotation_sensor->setDataRate(50);
                connect(local->m_rotation_sensor, SIGNAL(readingChanged()), local, SLOT(onRotationReadingChanged()));
                local->m_rotation_sensor->start();
                qDebug() << "Found a sensor QRotationSensor";
            }

            if(!strncmp(type,"QCompass",strlen("QCompass")))
            {
                local->m_compass_sensor = new QCompass();
                connect(local->m_compass_sensor, SIGNAL(readingChanged()), local, SLOT(onCompassReadingChanged()));
                local->m_compass_sensor->start();
                local->m_compass_sensor->setDataRate(50);
                qDebug() << "Found a sensor QCompass";
            }

            if(!strncmp(type,"QAccelerometer",strlen("QAccelerometer")))
            {
                local->m_accel_sensor = new QAccelerometer();
                local->m_accel_sensor->setDataRate(50);
                connect(local->m_accel_sensor, SIGNAL(readingChanged()), local, SLOT(onAccelerometerReadingChanged()));
                local->m_accel_sensor->start();
                qDebug() << "Found a sensor QAccelerometerReading";
            }

            if(!strncmp(type,"QGyroscope",strlen("QGyroscope")))
            {
                local->m_gyro_sensor = new QGyroscope();
                local->m_gyro_sensor->setDataRate(50);
                connect(local->m_gyro_sensor, SIGNAL(readingChanged()), local, SLOT(onGyroReadingChanged()));
                local->m_gyro_sensor->start();
                qDebug() << "Found a sensor QGyroscopeReading";
            }

            if(!strncmp(type,"QMagnetometer",strlen("QMagnetometer")))
            {
                local->m_mag_sensor = new QMagnetometer();
                local->m_mag_sensor->setDataRate(50);
                connect(local->m_mag_sensor, SIGNAL(readingChanged()), local, SLOT(onMagReadingChanged()));
                local->m_mag_sensor->start();
                qDebug() << "Found a sensor QMagnetometerReading";
            }
        }
        /*
        else{
            if(!strncmp(type,"QRotationSensor",strlen("QRotationSensor")))
            {
                local->m_rotation_sensor = new QRotationSensor();
                local->m_rotation_sensor->setDataRate(4);
                connect(local->m_rotation_sensor, SIGNAL(readingChanged()), local, SLOT(onRotationReadingChanged()));
                local->m_rotation_sensor->start();
                qDebug() << "Found a sensor QRotationSensor";
            }
        }
        */

    }
    // If we do not have an external IMU...
    if(local->m_use_imu  == false)
    {
        QString data = "IMU NOT found...";
        local->ui->listView->appendPlainText(data);
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString(FLIGHTLOG));
        if( l_file->open(QIODevice::ReadWrite | QIODevice::Append )){
            l_file->write(data.toLocal8Bit()+"\n");
            l_file->close();
        }
    }
    else{
        QString data = "IMU found and connected...";
        local->ui->listView->appendPlainText(data);
        local->ekf.m_use_gpt = 0; // 1; //0;
        QString x = local->ui->reset_att->styleSheet();
        x.replace(QString("1 ##888"), QString("1 #f00"));
        x.replace(QString("1 #0F0"),  QString("1 #f00"));
        local->ui->reset_att->setStyleSheet(x);
        local->ui->reset_att->update();
        QFile *l_file = new QFile(QString(LOG_DIR)+ QString(FLIGHTLOG));
        if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
        {
            l_file->write(data.toLocal8Bit()+"\n");
            l_file->close();
        }
    }

    qDebug() << mySensorList;

    if(local->ui->radioButton_3->isChecked() == false)
    {
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
    local->m_IMU->start(20);    
    local->m_dt = QDateTime::currentMSecsSinceEpoch();

#if defined(Q_OS_ANDROID) && defined(USE_KeepAwakeHelper)
    //local->helper = new KeepAwakeHelper();
    //local->helper->EnableKeepAwakeHelper();
#endif

//    local->m_msgBox->hide();
//    QCoreApplication::processEvents();
    local->m_msgBox->show();
    QCoreApplication::processEvents();
}

void MainWindow::permissionUpdated(const QPermission &permission)
{
    if (permission.status() != Qt::PermissionStatus::Granted)
    {
        qDebug() << "Precise location permission denied";
        return;
    }
    auto locationPermission = permission.value<QLocationPermission>();
    if (!locationPermission || locationPermission->accuracy() != QLocationPermission::Precise)
    {
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
    switch (qApp->checkPermission(cameraPermission))
    {
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
    for (const QCameraDevice &cameraDevice : availableCameras)
    {
        QAction *videoDeviceAction = new QAction(cameraDevice.description(), videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant::fromValue(cameraDevice));
        if (cameraDevice == QMediaDevices::defaultVideoInput())
            videoDeviceAction->setChecked(true);
    }
}

void MainWindow::setCamera(const QCameraDevice &cameraDevice)
{
#if defined(Q_OS_ANDROID) && defined(USE_KeepAwakeHelper)
    if(helper){
        delete helper;
        helper = nullptr;
    }
#endif

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
    connect(m_capture, &QImageCapture::imageCaptured,[this](int id, const QImage &preview)
        {
            qDebug() << "Image Captured...";
            ui->plainTextEdit->appendPlainText(QString("Image Captured:%1").arg(id));
            QDateTime date = QDateTime::currentDateTime();
            QString filename2 = IMAGES_DIR+QString("/")+date.toString("yyyy_dd_MM_hh_mm_ss").append(".jpg");
            QPixmap pixmap(QPixmap::fromImage(preview));
            QImage *imagex = new QImage(pixmap.toImage());
            imagex->save(filename2, "jpg", 100);
    /*
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(9);
    */
            /*
            QString filename1 = IMAGES_DIR+QString("/")+date.toString("yyyy_dd_MM_hh_mm_ss").append(".avi");
            imwrite("alpha2.png", frame, compression_params);
            VideoWriter video(filename1, CV_FOURCC('M','J','P','G'), 10, Size(qImageSingle.width(), qImageSingle.height()), true);
            for(int i=0; i<100; i++){
                video.write(frame); // Write frame to VideoWriter
            }
            */
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

#if defined(Q_OS_ANDROID) && defined(USE_KeepAwakeHelper)
//    helper->EnableKeepAwakeHelper();
#endif


}

void MainWindow::logTakeoff()
{
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(FLIGHTLOG));
    m_takeoffTime = QDateTime::currentDateTime();
    QString data = "Takeoff at: "+m_takeoffTime.toString();
    ui->listView->appendPlainText(data);

    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
    {
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

    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(FLIGHTLOG));
    if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
    {
        l_file->write(data.toLocal8Bit()+"\n");
        l_file->write(dtime.toLocal8Bit()+"\n");
        l_file->close();
    }
}

void MainWindow::doClock()
{
    static int tim = 0;

    if(!(++tim % 10)){
        static double lastTemp = -1;
        if(lastTemp != mysocket->Temp)
        {

            uint8_t major = (mysocket->VER  >> 8) & 0xFF;
            uint8_t minor = mysocket->VER  & 0xFF;
          //  printf("Firmware Version: %d.%d\n", major, minor);

            ui->plainTextEdit_2->setPlainText(
//            ui->plainTextEdit_2->appendPlainText(
//                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") +
                QString("IMU Ver: %1.%2\nTemp: %3").arg(major).arg(minor).arg( mysocket->Temp)
                );
            lastTemp = mysocket->Temp;
        }
    }

    ui->label_UTC->setText(QDateTime::currentDateTimeUtc().toString("hh:mm:ss"));
//      ui->label_UTC->setText(QDateTime::currentDateTime().toString("hh:mm:ss") );
    ui->timeEdit->setDateTime(QDateTime::currentDateTime());

    // If we are in the air...
    if(m_takeoff){
        ui->timeEdit_2->setTime(QTime::fromMSecsSinceStartOfDay(m_timer.elapsed()));
        // Have we landed...
        if(this->m_speed <= 5.0)
        {
            m_takeoff = false;
            logLanded();
        }
    }
    else{
        // If more than 30Km/t we are taking off...
        double alt;
        if(m_pressure_reader)
        {
            alt = m_preasure_alt;
        }
        else{
            if(m_altitude > 0.1) alt = m_altitude;
            else alt = 0;
        }

        if(this->ui->radioButton_2->isChecked()){
            if(m_altitude > 0.1) alt = m_altitude;
            else alt = 0;
        }

        if( this->m_speed > 20.0 && alt > (m_alt + 5) )
        {
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
    double var;

    if(m_pressure_reader)
    {
        m_vario = this->m_preasure_alt - vario;
        vario = this->m_preasure_alt;
        var=m_vario*60;  // Feet/m...
    }
    else{
        m_vario = this->m_altitude - vario;
        vario = this->m_altitude;
        var = m_vario *60;// Feet/m...
    }

    static double varfilter[varfilterlength]={0};
    double var_speed = 0;
    for(int x=0; x < varfilterlength-1;x++)
    {
        varfilter[x] = varfilter[x+1];
        var_speed+=varfilter[x];
    }
    varfilter[varfilterlength-1] = var;
    var_speed+=var;
    m_var_speed=var_speed/varfilterlength;
}

void MainWindow::onGyroReadingChanged()
{
/*
    static steady_clock::time_point clock_begin = steady_clock::now();
    steady_clock::time_point clock_end = steady_clock::now();
    steady_clock::duration time_span = clock_end - clock_begin;
    qDebug() << double(time_span.count()) / 1000000000L;
    clock_begin=clock_end;
*/
    if(!m_has_MQTT_gyro)
    {
        m_gyro_reader = m_gyro_sensor->reading();
        mysocket->AsX = m_gyro_reader->x();
        mysocket->AsY = m_gyro_reader->y();
        mysocket->AsZ = m_gyro_reader->z();
    }
}

void MainWindow::onMagReadingChanged()
{
    m_mag_reader = m_mag_sensor->reading();
    mysocket->HX = m_mag_reader->x();
    mysocket->HY = m_mag_reader->y();
    mysocket->HZ = m_mag_reader->z();
}

void MainWindow::onAccelerometerReadingChanged()
{
    if(!m_has_MQTT_accel)
    {
        m_accel_reader = m_accel_sensor->reading();
        mysocket->AccX = m_accel_reader->x();  // YAW
        mysocket->AccY = -m_accel_reader->y(); // ROLL
        mysocket->AccZ = -m_accel_reader->z(); // PITCH
    }
}

void MainWindow::onTempReadingChanged()
{
    m_temp_reader = m_temp_sensor->reading();
    m_temp = m_temp_reader->temperature();

}

void MainWindow::onCompassReadingChanged()
{
    if(!m_has_MQTT_heading)
    {
        m_compass_reader = m_compass_sensor->reading();
        //    m_heading = m_compass_reader->azimuth();
    }
}

void MainWindow::onOrientationReadingChanged()
{
    m_orientation_reader = m_orientation_sensor->reading();
    mysocket->Orient  = m_orientation_reader->orientation();// x();  // when rotated...
}

void MainWindow::onRotationReadingChanged()
{
    m_rotation_reader = m_rotation_sensor->reading();
    mysocket->AngleX  = -m_rotation_reader->x();        // Roll...
    mysocket->AngleY  = -m_rotation_reader->y()-90;     // Pitch...
    mysocket->AngleZ  = m_rotation_reader->z(); // Yaw...
 //   qDebug() << mysocket->AngleX << mysocket->AngleY << mysocket->AngleZ;
}

void MainWindow::AccelerometerRead()
{
#define GFILTER 100
    static double avg=9.82500f;
    double tmp=0;

    if(m_calibrate > 0)
    {
        tmp = sqrt(mysocket->AccX*mysocket->AccX +
                   mysocket->AccY*mysocket->AccY +
                   mysocket->AccZ*mysocket->AccZ);

        avg -= avg/GFILTER;
        avg += tmp/GFILTER;

        // qDebug() << "AVG G: " << avg;

        if(m_calibrate == 1)
        {
            qDebug() << "Saved...!!!!!!!!!!!!!!!";
            if(mysocket->IMUconnected == false){
#ifdef Q_OS_MAC
                m_first = 2;
#else
                m_first = 200;
#endif
            }
            else{
                m_first = 2;
            }
            // Store install orientation...
            m_install = Vector3d(0,-mysocket->AngleY*DEG_TO_RAD,0);

            mysocket->G         = avg;
            ekf.ekf->Gval       = avg;
            ekf.ekf_quart->Gval = avg;
            ekf.Gval            = avg;

            Matrix3x6 config;
            if(get_default_config(config) == 0)
            {
                config[0][2] = (double)avg;
                set_default_config(config);
                qDebug() << "Stored new config!!! " << avg;
            }
        }
        m_calibrate--;
    }
}

void MainWindow::onPressureReadingChanged()
{
    //    qDebug() << "  onPressureReadingChanged  ";
    static bool first = true;

    m_pressure_reader = m_pressure_sensor->reading();
    m_pressure_raw = m_pressure_reader->pressure()/100.0;
  //  qDebug() << m_pressure_raw;

  //  if( this->m_speed < 1.0){ first = true; }

    if( this->m_altitude > 0.1)
    {
        // Sett takeoff altitude...
        if(first)
        {
            first = false;
            m_alt = this->m_altitude;
            setQNH();
        }
    }

    // --- Runtime computation using the UI offset ---
    const double qnh_hpa_ui = ui->doubleSpinBox->text().toDouble();  // sea-level pressure
//    const double qnh_hpa_ui = m_pressure_raw + (ui->doubleSpinBox->text().toDouble() - 1013.25);  // sea-level pressure

    // If you want *indicated altitude* (baro matched to QNH):
    m_preasure_alt = 145366.45 * (1.0 - std::pow(m_pressure_raw / qnh_hpa_ui, 0.190284));
    ui->baro_alt->setText(QString::number(m_preasure_alt));
}

double MainWindow::setQNH()
{
    if( this->m_altitude > 0.1)
    {

        // Known altitude in feet (e.g. GPS)
        double feet_gps = this->m_altitude;

        // Calculate the sea-level pressure (QNH) in hPa
        double qnh_hpa = m_pressure_raw /
                         std::pow(1.0 - (feet_gps / 145366.45), 1.0 / 0.190284);

        // Pressure offset in millibars (hPa) to add to m_pressure_raw
        double pressure_offset = qnh_hpa - m_pressure_raw;

        //  mysocket->m_preasure_QNH = 145366.45 * (1.0 - std::pow(qnh_hpa / 1013.25, 0.190284));
        //  double test              = 145366.45 * (1.0 - std::pow(m_pressure_raw / 1013.25, 0.190284));

        // Now your calibrated pressure
   //     double calibrated_pressure = m_pressure_raw + pressure_offset;

        // Check: this should now yield GPS altitude
        mysocket->m_preasure_QNH  = 145366.45 * (1.0 - std::pow(m_pressure_raw / qnh_hpa, 0.190284));

        qDebug() << feet_gps << "  " << pressure_offset << "  "
                 << (m_pressure_raw + pressure_offset) << " qnh-> " << qnh_hpa
                 << "  " << mysocket->m_preasure_QNH   << " ---- " << mysocket->m_preasure_QNH; // << " Test: " << test;

        ui->doubleSpinBox->setText(QString("%1").arg(1013.25 + pressure_offset));
        return 1013.25 + pressure_offset;
    }
    else return 1013.25;
}

// Returns bearing in degrees (0° = North, 90° = East, etc.)
double MainWindow::getBearing(double lat1, double lon1, double lat2, double lon2)
{
    // Convert degrees to radians
    double φ1 = qDegreesToRadians(lat1);
    double φ2 = qDegreesToRadians(lat2);
    double Δλ = qDegreesToRadians(lon2 - lon1);

    double y = qSin(Δλ) * qCos(φ2);
    double x = qCos(φ1) * qSin(φ2) - qSin(φ1) * qCos(φ2) * qCos(Δλ);
    double θ = qAtan2(y, x);

    double bearing = qRadiansToDegrees(θ);
    return fmod((bearing + 360.0), 360.0); // Normalize to 0-360°
}

void MainWindow::positionUpdated(QGeoPositionInfo geoPositionInfo)
{
    bool has_pos = false;
    double vel_D=0;
    static bool first = true;
    static steady_clock::time_point clock_begin = steady_clock::now();
    steady_clock::time_point clock_end = steady_clock::now();
    steady_clock::duration time_span = clock_end - clock_begin;
    double dt = double(time_span.count()) / 1000000000L;
    clock_begin=clock_end;

    if(simGPS && !m_has_MQTT)
    {
        static bool first = true;
        static QList<TrackPoint> points;

        if(first == true)
        {
            first = false;
            GpxParser parser;
            if (parser.parseFile(":/sin.gpx"))
            {
//                if (parser.parseFile(":/example.gpx")) {
                points = parser.getTrackPoints();
            }
        }
        if(!points.isEmpty())
        {
            static double l_speed=0;
            static double l_delta_speed = 0.08;

            has_pos = true;
            TrackPoint pt = points.takeFirst();
            // Correct...
            m_gpsbearing      = getBearing(this->m_latitude,this->m_longitude,pt.latitude,pt.longitude);
            // Upside down for testing south direction...
  //          m_gpsbearing      = getBearing(pt.latitude,pt.longitude,this->m_latitude,this->m_longitude);
            this->m_latitude  = pt.latitude;
            this->m_longitude = pt.longitude;
            this->m_altitude  = pt.elevation*3.2808399;
            m_gpsspeed        = 25+(sin(l_speed)*20); //pt.speed;
            dt                = pt.dt;
            vel_D             = 0;

            l_speed+=l_delta_speed;
/*
            qDebug() << "Lat:" << pt.latitude
                     << "Lon:" << pt.longitude
                     << "Ele:" << pt.elevation
                     << "Speed:" << pt.speed
                     << "Bearing:" << m_gpsbearing
                     << "Time:" << pt.time.toString(Qt::ISODate);
*/
        }
        else{
            first = true;
        }
    }

    if (geoPositionInfo.isValid() && has_pos == false)
    {
        has_pos = true;
        //locationDataSource->stopUpdates();
        QGeoCoordinate geoCoordinate = geoPositionInfo.coordinate();
        this->m_latitude  = geoCoordinate.latitude();
        this->m_longitude = geoCoordinate.longitude();
        this->m_altitude  = (geoCoordinate.altitude()-40.0)*3.2808399; // BE AWARE THIS IS WGS-84 prox. compensated N AND NOT AMSL (see level) ...
    //    qDebug() << "GPS Alt:  " << this->m_altitude ;

        m_gpsspeed   = geoPositionInfo.attribute(QGeoPositionInfo::GroundSpeed);
        m_gpsbearing = geoPositionInfo.attribute(QGeoPositionInfo::Direction);
        vel_D        = -geoPositionInfo.attribute(QGeoPositionInfo::VerticalSpeed);
    }

    if(has_pos == true)
    {
        m_geopos = true;
        m_speed    = m_gpsspeed*3.6;

        if(m_gpsspeed > 2.5 && !isnan(m_gpsbearing))
        {
            static double old_bearing = 0;

            m_bearing    = m_gpsbearing;

            double delta_bearing = m_bearing - old_bearing;
            if (delta_bearing > 180) delta_bearing -= 360;
            if (delta_bearing < -180) delta_bearing += 360;

            double turn_rate_derivated = delta_bearing / dt;

            // Filter
            static double filtered_turn_rate = 0;
            double alpha = 0.1;
            filtered_turn_rate = alpha * turn_rate_derivated + (1 - alpha) * filtered_turn_rate;

            double a_c_exp = m_gpsspeed * filtered_turn_rate * DEG_TO_RAD;
            m_roll_angle = -atan2(a_c_exp, ekf.Gval);

            m_total_accel = sqrt(ekf.Gval); //sqrt((ekf.Gval*ekf.Gval)); // + (a_c_exp*a_c_exp)); //    # magnitude of net acceleration
            old_bearing = m_bearing;

            // correct for turn and acceleration...
            roll_blended = m_roll_angle;
            roll_blended_ok = true;

        }
        else{
            m_bearing     = 999;
            m_total_accel = 0;
            m_roll_angle  = 0;
            m_accel_body  = {0,0,0};
            roll_blended_ok = false;
        }
        // Get velocity vector...
        if(!(isnan(m_gpsspeed) || isnan(m_gpsbearing) || isnan(vel_D)))
        {
            this->m_vel_N  = cos(m_gpsbearing*DEG_TO_RAD)*m_gpsspeed;
            this->m_vel_E  = sin(m_gpsbearing*DEG_TO_RAD)*m_gpsspeed;
            this->m_vel_D  = vel_D;
            this->m_vel_active = true;
        }
        else{
            this->m_vel_E=this->m_vel_N=this->m_vel_D=0.0;
            this->m_vel_active = false;
        }

     //   ui->quickWidget->rootObject()->setProperty("lat", this->m_latitude);
     //   ui->quickWidget->rootObject()->setProperty("lon", this->m_longitude);
        /*
        qDebug() << "Lat: " << this->m_latitude << " Lon: " << this->m_longitude;
        qDebug() << "m_speed     = " << m_speed << Qt::endl;
        qDebug() << "m_latitude  = " << m_latitude << Qt::endl;
        qDebug() << "m_longitude = " << m_longitude << Qt::endl;
        qDebug() << "m_altitude  = " << m_altitude << Qt::endl;
        qDebug() << "m_head      = " << m_head << Qt::endl;
*/
        // set ground altitude...
        if(isnan(this->m_altitude))
        {
            this->m_altitude = 0;
        }else{
            // Set takeoff altitude...
            if(first && this->ui->radioButton_2->isChecked())
            {
                first = false;
                m_alt = this->m_altitude;
                // qDebug() << "set takeoff alt";
            }
            // As long as we do not move, but got altitude...
            if(m_gpsspeed < 0.5)
            {
                first = true;
                // qDebug() << "reset takeoff alt";
            }
        }
    }
    else{
        qDebug() << "No GPS data at: " << dt;
    }
}

void MainWindow::EKF()
{
    if((m_gyro_reader != nullptr && m_accel_reader != nullptr) || (ekf.m_use_gpt == 0) || m_use_imu )
    {
        static steady_clock::time_point clock_begin = steady_clock::now();
        steady_clock::time_point clock_end = steady_clock::now();
        steady_clock::duration time_span = clock_end - clock_begin;
        m_dt = double(time_span.count()) / 1000000000L;
        clock_begin=clock_end;

        // How to caulcate Euler Angles [Roll Φ(Phi) Gyro Z, Pitch θ(Theta) gyro Y, Yaw Ψ(Psi) Gyro X]
        if(ekf.m_use_gpt > 0)
        {
          //  if(m_mag_reader == nullptr){
          //      mysocket->HX=mysocket->HY=mysocket->HZ=0;
          //  }
            Vector3d gyro = {mysocket->AsZ,mysocket->AsY,-mysocket->AsX};
            Vector3d accel= {-mysocket->AccY,-mysocket->AccZ,mysocket->AccX};
            Vector3d mag  = {mysocket->HX,mysocket->HY,mysocket->HZ};

            // Compensate for mounting angle (only tilt).
            // Counter-rotate in the oppositt direction of the mounting found at start...
            gyro = ekf.ekf_quart->mountingRot(1,gyro, m_install);
            accel = ekf.ekf_quart->mountingRot(0,accel, -m_install);
            mag = ekf.ekf_quart->mountingRot(0,mag, -m_install);
        //    qDebug() << m_install[0]*RAD_TO_DEG << m_install[1]*RAD_TO_DEG << m_install[2]*RAD_TO_DEG;

            // remove forces calculated by GPS...
            if(m_use_gps_in_attitude)
            {
                accel = accel - m_accel_body;
            }

            float dummy;            

            // Estimate using accelerometers...
            std::tie(a_pitch,a_roll,dummy) = ekf.getPitchRoll(accel[1],accel[0],accel[2],ekf.Gval);
            m_yaw = ekf.getHeading(mag[1],mag[0],mag[2], ekf.getRoll_rad(),ekf.getPitch_rad());

            if(isnan(m_yaw)) m_yaw = 0;
            if(isnan(a_pitch)) a_pitch = 0;
            if(isnan(a_roll)) a_roll = 0;
            if(isnan(a_yaw)) a_yaw = 0;

            // Calculates X and Y relative distances in meters.
            double ypos = 0;
            double xpos = 0;
            if(!(isnan(this->m_latitude) || isnan(this->m_longitude)))
            {
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
            if (this->m_vel_active == true)
            {
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
                           gyro[0]* DEG_TO_RAD, //gyro[0] * DEG_TO_RAD,
                           -gyro[2]* DEG_TO_RAD, // gyro[2] * DEG_TO_RAD,
                           a_pitch,
                           a_roll,
                           m_yaw, //a_yaw, // m_yaw,
                           // Adding magnetometer will help on long turns, however they are unpredictable.
                           0, //m_pitch,
                           0, //m_roll,
                           0 //m_yaw
                        );

#define SCALE 1.00
            Vector3d attitude = {ekf.getRoll_rad(),ekf.getPitch_rad(),ekf.getHeading_rad()};
            m_attitude = attitude * RAD_TO_DEG;
            m_heading = ekf.getHeading_rad()* RAD_TO_DEG;

            if(roll_blended_ok)
            {
                // correct for turn ...
                double r = roll_blended = 0.95 * m_attitude[0] + 0.05 * roll_blended; // complementary filter
                m_attitude[0] = r;
           }
        }
        else{
            Vector3d attitude = {mysocket->AngleX,
                                 mysocket->AngleY,
                                 mysocket->AngleZ};
            m_attitude = attitude;
/*
            Vector3d attitude = {mysocket->AngleX*DEG_TO_RAD,
                                 mysocket->AngleY*DEG_TO_RAD,
                                 mysocket->AngleZ*DEG_TO_RAD};
            m_attitude = attitude * RAD_TO_DEG;
*/
            m_heading = mysocket->AngleZ;

        }
    }
}

void MainWindow::onReadingChanged()
{
#define filterlength 30
    static double filter[filterlength]={0};
#define rotfilterlength 20
//    static double rotfilter[rotfilterlength]={0};
#define headfilterlength 3
//    static double headfilter[headfilterlength]={0};
    double x_head = 0.0;
    double slipp = 0.0;
//    static bool running = false;

    // Sensor rotation...
    if(m_calibrate != 0)
    {
        AccelerometerRead();
    }

    // Run code...
    {
        Vector3d gyro = {mysocket->AsZ,mysocket->AsY,mysocket->AsX};
       // Vector3d accel= {-mysocket->AccY,-mysocket->AccZ,mysocket->AccX};
        Vector3d attitude  = m_attitude;

        Vector3d accel= {-mysocket->AccY,-mysocket->AccZ,mysocket->AccX};
        accel = ekf.ekf_quart->mountingRot(0,accel, -m_install);

     //   qDebug() << accel[0] << accel[1] << accel[2] << mysocket->G;

        if(m_first > 0)
        {
            m_first--;

#ifdef Q_OS_IOS
            if(m_first == 20){
                splash->finish(this); // Hides splash screen
                delete(splash);
            }
#endif
            /*
            if(m_first == 73){
                m_msgBox->hide();
                QCoreApplication::processEvents();
                m_msgBox->show();
                QCoreApplication::processEvents();
            }
*/
            if(m_first == 1){
                if(!this->m_msgBox->isHidden())
                {
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

                qDebug() << "TERJE:::::: " << m_offset;

                //m_pitch_cal+= a_pitch - m_pitch;
                //m_roll_cal+= a_roll - m_roll;
            }
        }

        if(m_bearing != 999){
            x_head = m_bearing;
        }
        else{
            x_head = m_heading - heading_offset;
        }
        slipp = (accel[0]-m_acc_Y_calib)*9.5;

        if(mysocket->Transponderstat == false &&  ui->radioButton->isCheckable())
        {
            ui->radioButton->setChecked(false);
            ui->radioButton->setCheckable(true);
            ui->radioButton_3->setChecked(false);
        }
        else if(mysocket->Transponderstat == true &&  !ui->radioButton->isCheckable())
        {
            ui->radioButton->setCheckable(true);
        }
/*
        // Make compass...
        //-----------------
        static double filtered_h = 0;  // Persistent filtered value
        double alpha_h = 0.01;            // Smoothing factor (0 < alpha < 1)

        double h = x_head;            // Current raw gyro reading
        filtered_h = alpha_h * h + (1 - alpha_h) * filtered_h;

        double head_dir = filtered_h; // This is your smoothed gyro reading
        //-----------------
*/
        /*
        double head_dir = 0;
        for(int x=0; x < headfilterlength-1;x++){
            headfilter[x] = headfilter[x+1];
            head_dir+=headfilter[x];
        }
        headfilter[headfilterlength-1] = x_head;
        head_dir+=x_head;
        head_dir/=headfilterlength;
        */
        m_head = x_head; //head_dir;
        //-----------------

        // Store data in local registers...
        double roll_att = 0;
        double pitch_att = 0;

        roll_att  = -attitude[0];
        pitch_att = 1 * (attitude[1] - m_offset);

        //-----------------
        static double filtered_r = 0;  // Persistent filtered value
        double alpha = 0.02;            // Smoothing factor (0 < alpha < 1)
        double r = gyro[2];            // Current raw gyro reading
        if(r <  0.5 && r > 0){alpha = 0.25;}
        if(r > -0.5 && r < 0){alpha = 0.25;}
        filtered_r = alpha * r + (1 - alpha) * filtered_r;
        double rot_speed = -filtered_r; // This is your smoothed gyro reading
        //-----------------

        /*
        double r = gyro[2];// *2; //((sin(attitude[2]*DEG_TO_RAD)*gyro[0])+(cos(attitude[2]*DEG_TO_RAD)*gyro[2]));
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
        */
        //-----------------

        // Make the turn bank ...
        double slippIndicator = 0;
        if(slipp >  16.0)slipp =  16.0;
        if(slipp < -16.0)slipp = -16.0;
        for(int x=0; x < filterlength-1;x++)
        {
            filter[x] = filter[x+1];
            slippIndicator+=filter[x];
        }
        filter[filterlength-1] = slipp;
        slippIndicator+=filter[filterlength-1];
        slippIndicator/=filterlength;

        // Radar...
        static int memory = 0;
        static std::deque<float> dq = {0};
        static std::deque<float> sq = {0};

        dq.push_back(20.0+(mysocket->rDist*3.0));
        sq.push_back(20.0+(mysocket->rSpeed*1.5));
        if(++memory >= 120)
        {
            dq.pop_front();
            sq.pop_front();
            memory = 120;
        }
        // .......


        if( currentIndex == 2)
        {
            static int xtimer=0;

            if(++xtimer%3 == 0)
            {
                if( ui->radioButton_3->isChecked()){
                    _widgetALT->setAltitude(m_preasure_alt);
                }else if( ui->radioButton_2->isChecked())
                {
                    _widgetALT->setAltitude(this->m_altitude);
                }else{
                    _widgetALT->setAltitude(m_tansALT);
                }
                _widgetALT->redraw();

                if(this->m_speed >= 0 && this->m_speed < 300)
                    _widgetASI->setAirspeed(this->m_speed);
                else
                    _widgetASI->setAirspeed(0);

                _widgetASI->redraw();

                _widgetHI->setHeading(abs(m_head-359.9));
                _widgetHI->redraw();
            }

            _widgetAI->setRoll ( roll_att );
            //_widgetAI->setRoll ( m_roll_angle*RAD_TO_DEG); // JUST FOR NOT; GPS ROLL...

            _widgetAI->setPitch( pitch_att );
            _widgetAI->redraw();

            if(xtimer%5 == 0)
            {
                _widgetTC->setTurnRate(rot_speed);
                _widgetTC->setSlipSkid(slippIndicator);
                _widgetTC->redraw();

                _widgetVSI->setClimbRate(m_var_speed);
                _widgetVSI->redraw();
            }
        }

        if( currentIndex == 6)
        {
            ui->label_Bearing->setText("10.0");
            ui->label_Bearing->raise();
            ui->label_Pitch->setText("11.0");
            ui->label_Pitch->raise();
            ui->label_Power->setText("12.0");
            ui->label_Power->raise();
            ui->label_Roll->setText("13.0");
            ui->label_Roll->raise();
            ui->label_Pitch->setText("14.0");
            ui->label_Pitch->raise();
            ui->label_UTC->setText("15.0");
            ui->label_UTC->raise();
            ui->label_TMP->setText("16.0");
            ui->label_TMP->raise();
            ui->label_Speed->setText("17.0");
            ui->label_Speed->raise();
        }

        if( currentIndex == 3)
        {
            if( ui->radioButton_3->isChecked())
            {
                _widgetEADI->setAltitude( m_preasure_alt );
            }else if( ui->radioButton_2->isChecked())
            {
                _widgetEADI->setAltitude(this->m_altitude);
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

        if( currentIndex == 4)
        {
            if(mysocket->Radarstat == true)
            {
                QGraphicsScene * m_graphScen = new QGraphicsScene;
                QSize x = ui->graphicsView_2->size();

                m_graphScen->setSceneRect(0,0,x.width(),x.height());
                m_graphScen->addLine((int)(10),(int)(5), (int)(10),(int)x.height(),QPen(QBrush(Qt::white),2));

                m_graphScen->addLine((int)(5),(int)(20), (int)(x.width()-0),(int)(20),QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(50), (int)(x.width()-0),(int)(50),QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(80), (int)(x.width()-0),(int)(80),QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(110), (int)(x.width()-0),(int)(110),QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(140), (int)(x.width()-0),(int)(140),QPen(QBrush(Qt::green),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(170), (int)(x.width()-0),(int)(170),QPen(QBrush(Qt::cyan),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(200), (int)(x.width()-0),(int)(200),QPen(QBrush(Qt::cyan),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(230), (int)(x.width()-0),(int)(230),QPen(QBrush(Qt::yellow),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(260), (int)(x.width()-0),(int)(260),QPen(QBrush(Qt::yellow),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(290), (int)(x.width()-0),(int)(290),QPen(QBrush(Qt::yellow),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(320), (int)(x.width()-0),(int)(320),QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));
                m_graphScen->addLine((int)(5),(int)(350), (int)(x.width()-0),(int)(350),QPen(QBrush(Qt::red),1,Qt::PenStyle(Qt::DashLine)));

                for( int i=0; i < memory; i++ )
                {
                    float height_pos = dq[i];
                    float speed_pos = sq[i];
                    //qDebug() << height_pos << speed_pos;

                    m_graphScen->addLine((int)(i*6),
                                         (int)x.height()-height_pos,
                                         (int)(i*6)+2,
                                         (int)x.height()-height_pos,
                                         QPen(QBrush(Qt::yellow),4));

                    m_graphScen->addLine((int)(i*6),
                                         (int)x.height()-speed_pos,
                                         (int)(i*6)+2,
                                         (int)x.height()-speed_pos,
                                         QPen(QBrush(Qt::white),4));
                }

                ui->graphicsView_2->setScene(m_graphScen);
                ui->graphicsView_2->show();
            }
        }

        if( currentIndex == 1)
        {
            QGraphicsScene * m_graphScen = new QGraphicsScene;
            QSize x = ui->graphicsView->size();

            m_graphScen->setSceneRect(0,0,x.width(),x.height());
            float x1 = cos(-roll_att/(180.0/3.1415));
            float y1 = sin(-roll_att/(180.0/3.1415));
         //   float z1 = sin(pitch_att/(180.0/3.1415));

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

            if(m_speed < 300 && m_speed > 0)
            {
                ui->speed->setText(QString("%1").arg(abs(m_speed), 0, 'f', 1));
            }

#ifndef Q_OS_MAC
            if(mysocket->IMUconnected == true)
            {
                ui->roll->setText(QString("%1").arg(abs(roll_att), 0, 'f', 0));
                ui->pitch->setText(QString("%1").arg((pitch_att), 0, 'f', 0));
                ui->temp->setText(QString("%1").arg(mysocket->AIR_PRESSURE, 0, 'f', 0));
                ui->temperature->setText(QString("%1").arg(mysocket->FW_Speed, 0, 'f', 1));
                ui->compass->setText(QString("%1").arg(m_head, 0, 'f', 0));
            }
            else if(m_rotation_sensor != nullptr)
#endif
            {
                ui->roll->setText(QString("%1").arg(abs(roll_att), 0, 'f', 0));
                ui->pitch->setText(QString("%1").arg((pitch_att), 0, 'f', 0));
                ui->compass->setText(QString("%1").arg(m_head, 0, 'f', 0));
                if(m_temp_sensor != nullptr) ui->temperature->setText(QString("%1").arg(m_temp, 0, 'f', 1));
            }

            if( m_geopos == true)
            {
                ui->altitude->setText(QString("%1").arg(m_altitude, 0, 'f', 0));
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
    if ( alt_receiced == false)
    {
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

void MainWindow::getVal(void *parent, const char *data, uint32_t length) //const QByteArray &array)
{
    static char buffer[30];
    static int pos = 0;
    MainWindow* saved_this= (MainWindow*) parent;
    Ui::SCREEN* local_ui  = saved_this->ui;

//    qDebug() << array;

    for(int i=0; i < length;i++)
    {
        if(data[i] == '*')
        {
            QString x = local_ui->pushButton_10->styleSheet();
            x.replace(QString("1 #900"), QString("1 #090"));
            local_ui->pushButton_10->setStyleSheet(x);
            local_ui->pushButton_10->update();
            saved_this->timerPing->stop();
            saved_this->timerPing->start(10000); // Turn off in 10 sec...
            qDebug() << "Ping received...";

            // Log all commands... This might be slow... will look at a timed write...
            QFile *l_file = new QFile(QString(LOG_DIR)+ QString(TRANSPONDERLOG));
            if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
            {
                QString data = QDateTime::currentDateTime().toString()+": "+"Ping received...\n";
                l_file->write(data.toLocal8Bit());
                l_file->close();
            }
        }

        if(data[i] < 0x1F || pos >= (int)sizeof(buffer))
        {
            if(pos >= 3)
            {
                // Log all commands... This might be slow... will look at a timed write...
                QFile *l_file = new QFile(QString(LOG_DIR)+ QString(TRANSPONDERLOG));
                if( l_file->open(QIODevice::ReadWrite | QIODevice::Append ))
                {
                    QString data = QDateTime::currentDateTime().toString()+": "+buffer+"\n";
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

                        switch(buffer[2])
                        {
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

                        if ( state == false)
                        {
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
                            {
                                if(buffer[key]=='M')
                                {
                                    number*=3.2808399;
                                    break;
                                }
                            }
                            saved_this->m_tansALT = round(number/100.0)*100;
                        }
                        else{
                            for (unsigned long key=0; key < strlen(buffer); key++)
                            {
                                if(buffer[key]=='F')
                                {
                                    number/=3.2808399;
                                    break;
                                }
                            }
                            saved_this->m_tansALT = round(number);
                            altType="Alt.M.";
                        }
                        snprintf(numout,20,"%.4d",(int)saved_this->m_tansALT);
                        local_ui->lcdNumber_3->display(numout);
                        local_ui->label_2->setText(altType);
//                        local_ui->baro_alt->setText(numout);
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
                            saved_this->timerPing->stop();
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
            if(data[i] >= 0x1F)
            {
                buffer[pos++]=data[i];
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

//-------------------------------------------------------------
//-------------------------------------------------------------
// Index 0 Transponder
void MainWindow::on_select_camera_from_transponder_clicked()
{
    setCamera(QMediaDevices::defaultVideoInput());
    currentIndex = 8;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_gyro_page_clicked()
{
    currentIndex = 1;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Index 1 Gyro eng
void MainWindow::on_select_transponder_page_clicked()
{
    if( mysocket->Transponderstat == true)
    {
        currentIndex = 0;
        ui->stackedWidget->setCurrentIndex(currentIndex);
    }
    else{
        setCamera(QMediaDevices::defaultVideoInput());
        currentIndex = 8;
        ui->stackedWidget->setCurrentIndex(currentIndex);
    }
}
void MainWindow::on_select_dumy_page2_clicked()
{
    currentIndex = 2;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Index 2 gyro nice
void MainWindow::on_select_transponder_page_2_clicked()
{
    currentIndex = 1;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_dumy_page2_2_clicked()
{
    currentIndex = 3;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Index 3 eadi
void MainWindow::on_select_transponder_page_3_clicked()
{
    currentIndex = 2;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_from_4_to_5_clicked()
{
    currentIndex = 4;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Index 4 Radar
void MainWindow::on_select_transponder_page_4_clicked()
{
    currentIndex = 3;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_from_5_to_6_clicked()
{
    currentIndex = 5;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Index 5 Radio list
void MainWindow::on_select_gyro_page2_clicked()
{
    currentIndex = 4;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_transponder_page2_clicked(){
    currentIndex = 6;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Index 6 Map
void MainWindow::on_select_gyro_page2_2_clicked()
{
    currentIndex = 5;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_transponder_page2_2_clicked()
{
    currentIndex = 7;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

// Indx 7 Config
void MainWindow::on_select_page2_map_clicked(){
    currentIndex = 6;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}
void MainWindow::on_select_transponder_page_camera_clicked(){
    setCamera(QMediaDevices::defaultVideoInput());
    currentIndex = 8;
    ui->stackedWidget->setCurrentIndex(currentIndex);

}

//Index 8 camera
void MainWindow::on_select_transponder_page2_3_clicked()
{
    hideCamera();
    if( mysocket->Transponderstat == true)
    {
        currentIndex = 0;
        ui->stackedWidget->setCurrentIndex(currentIndex);
    }
    else{
        currentIndex = 1;
        ui->stackedWidget->setCurrentIndex(currentIndex);
    }
}

void MainWindow::on_select_gyro_page2_3_clicked()
{
    hideCamera();
    currentIndex = 7;
    ui->stackedWidget->setCurrentIndex(currentIndex);
}

//-------------------------------------------------------------
//-------------------------------------------------------------

void MainWindow::on_imu_reset_clicked()
{
    m_calibrate = 100;
    m_msgBoxCalibrating = new NoButtonMessageBox(tr("Full Calibration\nPlease make sure the equipment is still !"));
    m_msgBoxCalibrating->show();
}

void MainWindow::on_use_hw_clicked()
{
    m_first = 10;

    m_msgBoxCalibrating = new NoButtonMessageBox(tr("Calibrating\nPlease make sure the equipment is still !"));
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


void MainWindow::on_textEdit_1_textChanged()
{
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(RADIO));
    if( l_file->open(QIODevice::ReadWrite ))
    {
        QString data = ui->textEdit->toPlainText();
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }
}


void MainWindow::on_textEdit_2_textChanged()
{
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(AIRPLANE));
    if( l_file->open(QIODevice::ReadWrite ))
    {
        QString data = ui->textEdit_2->toPlainText();
        l_file->write(data.toLocal8Bit());
        l_file->close();
    }

}

int MainWindow::get_default_config(Matrix3x6 &sensor)
{
    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(CONFIG));
    if (l_file->open(QIODevice::ReadOnly))
    {
        QString blob = l_file->readAll();
        l_file->close();

        QStringList pieces = blob.split( "\n");
        if(pieces.length() > 1)
        {
            static int found = 0;
            for(QString value : pieces)
            {
                QStringList element = value.split(",");

                if(element[0] == "AccelCal")
                {
                    sensor[0][0] = element[2].toDouble();
                    sensor[0][1] = element[4].toDouble();
                    sensor[0][2] = element[6].toDouble();
                    sensor[0][3] = element[8].toDouble();
                    sensor[0][4] = element[10].toDouble();
                    sensor[0][5] = element[12].toDouble();
                    found++;
                }
                if(element[0] == "GyroCal")
                {
                    sensor[1][0] = element[2].toDouble();
                    sensor[1][1] = element[4].toDouble();
                    sensor[1][2] = element[6].toDouble();
                    sensor[1][3] = element[8].toDouble();
                    sensor[1][4] = element[10].toDouble();
                    sensor[1][5] = element[12].toDouble();
                    found++;
                }
                if(element[0] == "MagCal")
                {
                    sensor[2][0] = element[2].toDouble();
                    sensor[2][1] = element[4].toDouble();
                    sensor[2][2] = element[6].toDouble();
                    sensor[2][3] = element[8].toDouble();
                    sensor[2][4] = element[10].toDouble();
                    sensor[2][5] = element[12].toDouble();
                    found++;
                }
/*
                if(element[0] == "TRANSPONDER")
                {
                    _transponder_id = element[1];
                    found++;
                }
                if(element[0] == "RADAR")
                {
                    _radar_id = element[1];
                    found++;
                }
                if(element[0] == "IMU")
                {
                    _IMU_id = element[1];
                    found++;
                }
*/
                /*
                for(QString val : element){
                    qDebug()<< val;
                }
                */
            }
            if(found == 6) return 0;
        }
        return -2;
    }
    return -1;
}

int MainWindow::set_default_config(const Matrix3x6 &sensor)
{
    QString txt = "AccelCal,X1,%1,Y1,%2,Z1,%3,X2,%4,Y2,%5,Z2,%6\n";
    txt.append(   "GyroCal,X,%7,Y,%8,Z,%9,X2,%10,Y2,%11,Z2,%12\n");
    txt.append(   "MagCal,X,%13,Y,%14,Z,%15,X2,%16,Y2,%17,Z2,%18\n");
    txt.append(   "TRANSPONDER,%19\n");
    txt.append(   "RADAR,%20\n");
    txt.append(   "IMU,%21\n");

    QString data  =  txt.arg(sensor[0][0]).arg(sensor[0][1]).arg(sensor[0][2]).arg(sensor[0][3]).arg(sensor[0][4]).arg(sensor[0][5])
                        .arg(sensor[1][0]).arg(sensor[1][1]).arg(sensor[1][2]).arg(sensor[1][3]).arg(sensor[1][4]).arg(sensor[1][5])
                        .arg(sensor[2][0]).arg(sensor[2][1]).arg(sensor[2][2]).arg(sensor[2][3]).arg(sensor[2][4]).arg(sensor[2][5])
                        .arg(_transponder_id).arg(_radar_id).arg(_IMU_id);


    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(CONFIG));
    if( l_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        l_file->write(data.toLocal8Bit());
        l_file->close();
        return 0;
    }
    return -1;
}

int MainWindow::set_default_radio()
{
    ui->textEdit->setText("121.500\tEmergency\n");
    ui->textEdit->insertPlainText("123.500\tStandard\n");
    ui->textEdit->insertPlainText("122.500\tNAK\n");
    ui->textEdit->insertPlainText("123.450\tPlane to Plane\n");
    ui->textEdit->insertPlainText("119.100\tKjeller\n");
    ui->textEdit->insertPlainText("122.700\tTorsnes\n");
    ui->textEdit->insertPlainText("118.470\tOslo Approache\n");

    QFile *l_file = new QFile(QString(LOG_DIR)+ QString(RADIO));
    qDebug() << "FILE: " << l_file->fileName();
    if( l_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString data = ui->textEdit->toPlainText();
        qDebug() << data;
        ui->plainTextEdit->appendPlainText(data);
        l_file->write(data.toLocal8Bit());
        l_file->close();
        return 0;
    }
    return -1;
}

int MainWindow::set_default_planes()
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
    if( l_file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString data = ui->textEdit_2->toPlainText();
        qDebug() << data;
        ui->plainTextEdit->appendPlainText(data);
        l_file->write(data.toLocal8Bit());
        l_file->close();
        return 0;
    }
    return -1;
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
    heading_offset = m_head; //mysocket->AngleZ;

    //    heading_offset = 180.0 - mysocket->AngleZ;
    //    if(heading_offset<0.0) heading_offset+=360.0;

    if(m_compass_sensor){
        //      m_compass_reader->setAzimuth(heading_offset);
    }
}

void MainWindow::on_reset_att_clicked()
{
    qDebug() << "Use IMU clicked... " << ekf.m_use_gpt;

    QString x = ui->reset_att->styleSheet();
    qDebug() << x;

    if(ekf.m_use_gpt == 0)
    {
        ekf.m_use_gpt = 1;
        m_rotation_sensor->setDataRate(4);
        x.replace(QString("1 #f00"), QString("1 #00F"));
        ui->reset_att->setText("EKF_1");
    }
    else if(ekf.m_use_gpt == 1)
    {
        if(m_use_imu) ekf.m_use_gpt = 2;
        else ekf.m_use_gpt = 3;
        m_rotation_sensor->setDataRate(4);
        x.replace(QString("1 #00F"), QString("1 #0F0"));
        ui->reset_att->setText("EKF_2");
    }
    else if(ekf.m_use_gpt == 2)
    {
        ekf.m_use_gpt = 3;
        m_rotation_sensor->setDataRate(4);
        x.replace(QString("1 #00F"), QString("1 #0F0"));
        ui->reset_att->setText("EXT.");
    }
    else{
        ekf.m_use_gpt = 0;
        m_rotation_sensor->setDataRate(50);
        x.replace(QString("1 #0F0"), QString("1 #f00"));
        ui->reset_att->setText("INT.");

    }
    qDebug() << x;
    ui->reset_att->setStyleSheet(x);
    ui->reset_att->update();
}

void MainWindow::on_fly_home_clicked()
{
    static bool pingpong=true;

    qDebug() << "FlyHome... ";
  //  ui->quickWidget->rootObject()->setProperty("zoomLevel", 5); // 18);

    if(pingpong){
        ui->quickWidget->raise();
        ui->fly_home->raise();
        ui->select_transponder_page2_2->raise();
        ui->select_gyro_page2_2->raise();
        pingpong=false;
    }else{
        ui->graphicsView_3->raise();
        ui->fly_home->raise();
        ui->select_transponder_page2_2->raise();
        ui->select_gyro_page2_2->raise();
        pingpong=true;
    }
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

void MainWindow::on_use_gps_in_attitude_clicked()
{
    static bool pressed = false;

    if(pressed)
    {
        pressed = false;
        m_use_gps_in_attitude = false;
    }
    else{
        pressed = true;
        m_use_gps_in_attitude = true;
    }
}

void MainWindow::on_reset_altitude_2_clicked()
{
    double pressure_mb = setQNH()*100.0;
    on_dial_valueChanged(pressure_mb);
    qDebug() << "Altitutde. " << pressure_mb;
}

void MainWindow::on_reset_altitude_3_clicked()
{
    if(m_takeoff == false) on_reset_altitude_2_clicked();
    qDebug() << "Alt 2. ";
}

void MainWindow::on_reset_heading_2_clicked()
{
    qDebug() << "Heading. ";

}

void MainWindow::on_use_built_inn_barometer_clicked()
{
    static bool active = false;
    QString x = ui->use_built_inn_barometer->styleSheet();

    if(active == false)
    {
        active = true;
        mysocket->TransponderstatWithBarometer = true;
        x.replace(QString("1 #080"), QString("1 #800"));
        ui->use_built_inn_barometer->setText("Use Built In\nbarometer");

    }
    else{
        active = false;
        mysocket->TransponderstatWithBarometer = false;
        x.replace(QString("1 #800"), QString("1 #080"));
        ui->use_built_inn_barometer->setText("Use External\nbarometer");
    }

    ui->use_built_inn_barometer->setStyleSheet(x);
    ui->use_built_inn_barometer->update();
}
