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

#include <QPressureReading>
#include <QGeoPositionInfo>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
//#include <QAltimeterReading>

#include "mytcpsocket.h"

#ifdef Q_OS_ANDROID
#include "ui_mainwindow_port_new.h"
#define SCREEN MainWindow_port_new
#include "lockhelper.h"
#endif

#ifdef Q_OS_IOS
#include "ui_mainwindow_port_iPhone.h"
#define SCREEN MainWindow_port_iPhone
#endif

#ifdef Q_OS_MAC
#include "ui_mainwindow_port_new.h"
#define SCREEN MainWindow_port_new
#endif

//#include "ui_mainwindow_phone.h"
//#include "ui_mainwindow_port_small.h"
//#include "ui_mainwindow_small.h"
//#define SCREEN MainWindow_port_small
//#define SCREEN MainWindow_small

QT_BEGIN_NAMESPACE
namespace Ui { class SCREEN; }
QT_END_NAMESPACE

#define RADIO "/setup_radio_b.txt"
#define AIRPLANE "/setup_ln_b.txt"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addnext(int x);
    void addcurrent(int x);
    void setmode(int mode);
    void set_default_radio(void);
    void set_default_planes(void);
    void logLanded();
    void logTakeoff();
    void init();
    void updateCameras();
    void hideCamera();
    void setCamera(const QCameraDevice &cameraDevice);
    //    void updateCameraActive(bool active);

    void permissionUpdated(const QPermission &permission);

    Qt::ScreenOrientation ScreenMode;

    int next[4]   ={7,0,0,0};
    int current[4]={8,8,8,8};
    int mode=0;
    double m_altitude=0;
    double m_latitude=0;
    double m_longitude=0;
    double m_speed=0;
    double m_head =9999;
    qreal m_temp =9999;

    QDateTime m_takeoffTime;
    QDateTime m_landedTime;

    MyTcpSocket *mysocket = NULL;

    static void getLidar(QByteArray);
    static void getVal(QByteArray);
    void onResized(int);
    QScreen* getActiveScreen(QWidget* pWidget) const;

    std::variant < Ui::SCREEN > xxz3 = *new (Ui::SCREEN);
    Ui::SCREEN *ui = (Ui::SCREEN *) &xxz3;

    QQuickView view;
    QSplineSeries *series;
    QElapsedTimer m_timer;

#ifdef Q_OS_ANDROID
    KeepAwakeHelper helper;
#endif

private:
    QActionGroup *videoDevicesGroup = nullptr;
    QMediaDevices m_devices;
    QScopedPointer<QCamera> m_camera;
    QMediaCaptureSession m_captureSession;
    QMediaRecorder *m_recorder;
    QCameraDevice *m_cameraDevic;

    bool alt_receiced;
    QTimer* m_Clock;
    QTimer* timerAlt;
    QTimer* timerPing;
    QTimer* timerActive;
    QTimer* timerpaint;
    QTimer *timertakePicture;
    QTimer *m_IMU;

    int alt_mode = 1;
    // const QRect *m_vsize;
    QSize *m_size;
    void setalt(int alt_mode);

    int m_reading = 0;
    bool m_first = true;
    bool m_armed = false;
    bool m_takeoff = false;

    //   QAltimeterSensor*  m_altimeter_sensor;
    QPressureSensor*  m_pressure_sensor = NULL;
    QPressureReading* m_pressure_reader = NULL;
    QGeoPositionInfoSource* m_geoPositionInfo = NULL;

    qreal m_offset;
    qreal m_alt = 0.0;
    double heading_offset=0.0;
    bool m_geopos = false;


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
    void on_reconnect_clicked();
    void on_pushButton_20_clicked();
    void on_reset_heading_clicked();


    void doCheck();
    void reset_ping();
    void active_ping();
    void doClock();

    void onRotationReadingChanged();
    void onPressureReadingChanged();
    void positionUpdated(QGeoPositionInfo geoPositionInfo);

    void on_select_gyro_page_clicked();
    void on_select_gyro_page2_clicked();
    void on_select_dumy_page_clicked();
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

    void takePicture();
    void on_pushButton_23_clicked();

    void on_exit_2_clicked();

public:
    int screen_index;

};
#endif // MAINWINDOW_H
