#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickView>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QElapsedTimer>
#include <QCameraDevice>
#include <QMediaDevices>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMessageBox>

#include <QGyroscopeReading>
#include <QGyroscope>

#include <QAccelerometerReading>
#include <QAccelerometer>

#include <QCompassReading>
#include <QCompass>

#include <QMagnetometerReading>
#include <QMagnetometer>

#include <QOrientationSensor>
#include <QOrientationReading>

#include <QAmbientTemperatureSensor>
#include <QAmbientTemperatureReading>

#include <QRotationSensor>
#include <QPressureReading>
#include <QGeoPositionInfo>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>

#include "mytcpsocket.h"
#include "ekfNavINS.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SCREEN; }
QT_END_NAMESPACE

// + "/Resources/"

#ifdef Q_OS_IOS
    #define LOG_DIR QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
    #define IMAGES_DIR QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
#else
    #define IMAGES_DIR "/storage/emulated/0/DCIM/Camera"
    #define LOG_DIR "/storage/emulated/0/Documents"
#endif

#define RADIO     "/setup_radio_b.txt"
#define AIRPLANE  "/setup_ln_b.txt"
#define CONFIG    "/config_b.txt"
#define FLIGHTLOG "/flightlog.txt"

// Define the Qiskit interface...
void Qiskit(void);


class NoButtonMessageBox : public QDialog {
public:
    NoButtonMessageBox(const QString &message, QWidget *parent = nullptr)
        : QDialog(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        setModal(true);

        auto layout = new QVBoxLayout(this);
        QLabel *label = new QLabel(message, this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("QLabel { font-size: 18pt; color: white; background-color: #333; padding: 20px; border-radius: 12px; }");

        layout->addWidget(label);
        setLayout(layout);
        resize(300, 100);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addnext(int x);
    void addcurrent(int x);
    void setmode(int mode);
    int  set_default_radio(void);
    int  set_default_planes(void);
    int  set_default_config(const Matrix3x6 &sensor);
    int  get_default_config(Matrix3x6 &sensor);
//    void set_default_config(double ax,double ay,double az,double gx,double gy,double gz,double hx,double hy,double hz);
    void logLanded();
    void logTakeoff();
    void init();
    void updateCameras();
    void hideCamera();
    void setCamera(const QCameraDevice &cameraDevice);
    void permissionUpdated(const QPermission &permission);
    void setButtonIcon(QString iconPath, QPushButton* button);


    //    void updateCameraActive(bool active);
    Vector3x m_attitude;

    Matrix3x3 rotationMatrix;

    Qt::ScreenOrientation ScreenMode;

    int next[4]   ={7,0,0,0};
    int current[4]={8,8,8,8};
    int mode=0;

    double m_a_min_x = 0, m_a_min_y = 0, m_a_min_z = 0;
    double m_a_max_x = 0, m_a_max_y = 0, m_a_max_z = 0;
    double m_m_min_x = 0, m_m_min_y = 0, m_m_min_z = 0;
    double m_m_max_x = 0, m_m_max_y = 0, m_m_max_z = 0;

    double m_altitude      = 0;
    double m_latitude      = 0;
    double m_longitude     = 0;

    double m_preasure_alt  = 0;
    double m_preasure      = 0;

    double takeoff_latitude  = 0;
    double takeoff_longitude = 0;
    double takeoff_altitude  = 0;

    double m_gpsspeed = 0;
    double m_gpsbearing = 0;
    double m_vel_N  = 0;
    double m_vel_E  = 0;
    double m_vel_D  = 0;
    bool m_vel_active = false;

    double m_tansALT       = 0;

    bool m_use_imu = false;

    double m_speed=0;
    double m_head =9999;
    qreal m_temp  =9999;

    double m_var_speed = 0;
    double m_ms = 0;

    QDateTime m_takeoffTime;
    QDateTime m_landedTime;

    MyTcpSocket *mysocket = NULL;
    int currentIndex = 0;

    static void setIMU(void *parent, bool use_imu);
    static void getVal(void *parent, QByteArray);
    void onResized(int);
    QScreen* getActiveScreen(QWidget* pWidget) const;


    QQuickView view;
    QSplineSeries *series;
    QElapsedTimer m_timer;

    WidgetAI  *_widgetAI;
    WidgetAI  * getAI   () { return _widgetAI; }
    WidgetTC  *_widgetTC;
    WidgetTC  * getTC   () { return _widgetTC; }
    WidgetALT  *_widgetALT;
    WidgetALT  * getALT () { return _widgetALT;}
    WidgetASI  *_widgetASI;
    WidgetASI  * getASI () { return _widgetASI;}
    WidgetVSI  *_widgetVSI;
    WidgetVSI  * getVSI () { return _widgetVSI;}
    WidgetHI  *_widgetHI;
    WidgetHI  * getHI   () { return _widgetHI; }
    WidgetEADI  *_widgetEADI;
    WidgetEADI  * getEADI() { return _widgetEADI; }
    WidgetEHSI  *_widgetEHSI;
    WidgetEHSI  * getEHJSI() { return _widgetEHSI; }

private:
    QActionGroup *videoDevicesGroup = nullptr;
    QMediaDevices m_devices;
    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_captureSession;
    QMediaRecorder *m_recorder;
    QCameraDevice *m_cameraDevic;
    QImageCapture *m_capture;

    bool alt_receiced;
    QTimer* m_Clock;
    QTimer* timerAlt;
    QTimer* timerPing;
    QTimer* timerActive;
    QTimer* timerpaint;
    QTimer *timertakePicture;
    QTimer *m_IMU;
    QTimer *m_Display;

    ekfNavINS ekf;

    int alt_mode     = 1;
    // const QRect *m_vsize;
    QSize *m_size;
    void setalt(int alt_mode);

    int m_reading    = 0;
    int m_first      = 75;        // Count down to recalibrate...
    int m_calibrate  = 0;
    bool m_armed     = false;
    bool m_takeoff   = false;
    double m_bearing = 999;
    double m_heading = 0;

    //   QAltimeterSensor*  m_altimeter_sensor;
    QPressureSensor*    m_pressure_sensor = NULL;
    QPressureReading*   m_pressure_reader = NULL;

    QOrientationSensor*  m_orientation_sensor  = nullptr;
    QOrientationReading*  m_orientation_reader = nullptr;

    //   QAltimeterSensor*  m_altimeter_sensor;
    QRotationSensor*    m_rotation_sensor = nullptr;
    QRotationReading*   m_rotation_reader = nullptr;

    QCompass*           m_compass_sensor  = nullptr;
    QCompassReading*    m_compass_reader  = nullptr;

    QAccelerometer*     m_accel_sensor    = nullptr;
    QAccelerometerReading* m_accel_reader = nullptr;
    double              m_acc_Y_calib     = 0.0;

    QGyroscope*         m_gyro_sensor     = nullptr;
    QGyroscopeReading*  m_gyro_reader     = nullptr;

    QMagnetometer*        m_mag_sensor    = nullptr;
    QMagnetometerReading* m_mag_reader    = nullptr;

    QAmbientTemperatureSensor*  m_temp_sensor    = nullptr;
    QAmbientTemperatureReading* m_temp_reader    = nullptr;

    QGeoPositionInfoSource* m_geoPositionInfo = NULL;

    qreal m_offset      = 0.0;
    qreal m_alt         = 0.0;
    double heading_offset = 0.0;
    bool m_geopos       = false;
    double m_head_dir   = 0.0;
    double m_dt;

    double a_pitch      = 0.0;
    double a_roll       = 0.0;
    double m_pitch_cal  = 0.0;
    double m_roll_cal   = 0.0;
    double m_pitch      = 0.0;
    double m_roll       = 0.0;
    double m_yaw        = 0.0;


    NoButtonMessageBox *m_msgBox = nullptr;
    NoButtonMessageBox *m_msgBoxCalibrating = nullptr;

    // Q_SIGNALS:
    void accepted();

private slots:
    void setVal();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_19_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_17_clicked();
    void on_pushButton_16_clicked();
    void on_pushButton_18_clicked();
    void on_pushButton_Ident_clicked();
    void on_pushButton_stby_clicked();
    void on_pushButton_off_clicked();
    void on_pushButton_norm_clicked();
    void on_pushButton_alt_clicked();
    void on_pushButton_12_clicked();
    void on_pushButton_13_clicked();
    void on_reconnect_now_clicked();
    void on_pushButton_20_clicked();
    void on_reset_heading_clicked();
    void on_select_transponder_page_3_clicked();
    void on_select_from_4_to_5_clicked();

    void doCheck();
    void reset_ping();
    void active_ping();
    void doClock();

    void positionUpdated(QGeoPositionInfo geoPositionInfo);

    void onRotationReadingChanged();
    void onReadingChanged();
    void onPressureReadingChanged();
    void onCompassReadingChanged();
    void onAccelerometerReadingChanged();
    void onGyroReadingChanged();
    void onMagReadingChanged();
    void onOrientationReadingChanged();
    void onTempReadingChanged();

    void on_select_gyro_page_clicked();
    void on_select_gyro_page2_clicked();
    void on_select_camera_from_transponder_clicked();
    void on_select_dumy_page2_clicked();
    void on_select_transponder_page_clicked();
    void on_select_transponder_page2_clicked();
    void on_imu_reset_clicked();
    void on_timer_start_clicked();
    void on_textEdit_textChanged();
    void on_textEdit_2_textChanged();
    void on_pushButton_15_clicked();
    void on_select_transponder_page2_2_clicked();
    void on_select_gyro_page2_2_clicked();
    void on_select_transponder_page2_3_clicked();
    void on_select_gyro_page2_3_clicked();
    void on_pushButton_21_clicked();
    void on_pushButton_22_clicked();
    void on_select_dumy_page2_2_clicked();
    void on_select_transponder_page_2_clicked();
    void takePicture();
    void on_pushButton_23_clicked();
    void on_exit_2_clicked();
    void on_use_hw_clicked();
    void on_reset_att_clicked();
    void on_fly_home_clicked();
    void on_dial_valueChanged(int value);
    void on_dial_2_valueChanged(int value);
    void EKF();


public:
    int screen_index;

    Ui::SCREEN *ui = (Ui::SCREEN *) &(*new (Ui::SCREEN));
};
#endif // MAINWINDOW_H
