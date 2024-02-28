#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickView>
#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>

#include <QGyroscope>
#include <qgyroscope.h>
#include <QAccelerometer>
#include <qaccelerometer.h>
#include <QRotationSensor>
#include <qrotationsensor.h>
#include <QPressureReading>
//#include <QAltimeterReading>

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
    bool m_takeoff = false;

 //   QAltimeterSensor*  m_altimeter_sensor;

    QPressureSensor*  m_pressure_sensor;
    QPressureReading* m_pressure_reader;
    QRotationSensor*  m_rotation_sensor;
    QRotationReading* m_rotation_reader;
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
    void on_pushButton_20_clicked();

public:
    int screen_index;

    std::variant<Ui::MainWindow_port_small, Ui::MainWindow_port, Ui::MainWindow_small,Ui::MainWindow > *xxz2;
    std::variant<Ui::MainWindow_port_small, Ui::MainWindow_port, Ui::MainWindow_small,Ui::MainWindow >getauto();

};
#endif // MAINWINDOW_H
