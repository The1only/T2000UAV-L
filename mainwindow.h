#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickView>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QElapsedTimer>

#ifdef Q_OS_ANDROID
#include <QGyroscope>
#include <qgyroscope.h>
#include <QAccelerometer>
#include <qaccelerometer.h>
#include <QRotationSensor>
#include <qrotationsensor.h>
#include <QPressureReading>
#include <QCompassReading>
#include <QGeoPositionInfo>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
//#include <QAltimeterReading>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow_port; class MainWindow; class MainWindow_small; class MainWindow_port_small; }
QT_END_NAMESPACE

#include "mytcpsocket.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addnext(int x);
    void addcurrent(int x);
    void setmode(int mode);
    Qt::ScreenOrientation ScreenMode;

    int next[4]   ={7,0,0,0};
    int current[4]={8,8,8,8};
    int mode=0;
    double m_altitude=9999;
    double m_latitude=0;
    double m_longitude=0;
    double m_speed=NULL;
    double m_head=NULL;

    MyTcpSocket *mysocket = NULL;
    static void getLidar(QByteArray);
    static void getVal(QByteArray);
    void onResized(int);
    QScreen* getActiveScreen(QWidget* pWidget) const;
    Ui::MainWindow_small *ui;
    QQuickView view;
    QSplineSeries *series;

    QElapsedTimer m_timer;


private:
    bool alt_receiced;
    QTimer* m_Clock;
    QTimer* timerAlt;
    QTimer* timerPing;
    QTimer* timerpaint;
    int alt_mode = 1;
    void setalt(int alt_mode);

    int m_reading = 0;
    bool m_first = true;
    bool m_armed = false;
    bool m_takeoff = false;

 #ifdef Q_OS_ANDROID
 //   QAltimeterSensor*  m_altimeter_sensor;
    QPressureSensor*  m_pressure_sensor = NULL;
    QPressureReading* m_pressure_reader = NULL;
    QRotationSensor*  m_rotation_sensor = NULL;
    QRotationReading* m_rotation_reader = NULL;
    QCompass*         m_compass_sensor = NULL;
    QCompassReading*  m_compass_reader = NULL;
    QGeoPositionInfoSource* m_geoPositionInfo = NULL;
#endif

    qreal m_offset;
    qreal m_alt;

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

    void doAlt();
    void reset_ping();
    void doClock();

    void onRotationReadingChanged();
    void onPressureReadingChanged();
    void onCompassReadingChanged();

#ifdef Q_OS_ANDROID
    void positionUpdated(QGeoPositionInfo geoPositionInfo);
#endif

    void on_select_gyro_page_clicked();
    void on_select_gyro_page2_clicked();
    void on_select_dumy_page_clicked();
    void on_select_dumy_page2_clicked();
    void on_select_transponder_page_clicked();
    void on_select_transponder_page2_clicked();



    void on_imu_reset_clicked();

    void on_timer_start_clicked();

public:
    int screen_index;

    std::variant<Ui::MainWindow_port_small, Ui::MainWindow_port, Ui::MainWindow_small,Ui::MainWindow > *xxz2;
    std::variant<Ui::MainWindow_port_small, Ui::MainWindow_port, Ui::MainWindow_small,Ui::MainWindow >getauto();

};
#endif // MAINWINDOW_H
