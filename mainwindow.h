#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QQuickView>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow_port; class MainWindow; class MainWindow_small; }
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
    int current[4]={7,0,0,0};
    int mode=0;

    MyTcpSocket *mysocket = NULL;
    static void getVal(QByteArray);
    void onResized(int);
    QScreen* getActiveScreen(QWidget* pWidget) const;


    QQuickView view;

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

private:
    Ui::MainWindow_port *ui2;
    Ui::MainWindow *ui;
    Ui::MainWindow_small *ui_amall;
};
#endif // MAINWINDOW_H
