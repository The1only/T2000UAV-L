#include <QtWidgets/qapplication.h>
#include <QtCore/QLoggingCategory>
#include <QApplication>
#include <QSplashScreen>
#include <QThread>

#include "mainwindow.h"
#include "myNativeWrapperFunctions.h"


int main(int argc, char *argv[])
{
    //Qt::ScreenOrientation ScreenMode;
    QApplication app(argc, argv);
/*
#if defined(Q_OS_IOS)
    QPixmap splashPixmap(":/splash.png");  // Or use a file path
    QSplashScreen splash(splashPixmap);
    splash.showMessage("Initializing FlightIMU...", Qt::AlignTop | Qt::AlignCenter, Qt::black);
    splash.show();
#endif
*/
    MainWindow d;

#if defined(Q_OS_IOS)
//    QTimer::singleShot(4000, &splash, [&]() {
//        splash.finish(&d); // Hides splash screen
//        d.m_msgBox->show();
//    });
#else
//    QTimer::singleShot(4000, &splash, [&]() {
//        splash.finish(&d); // Hides splash screen
//        d.m_msgBox->show();
//    });
#endif

//    app.processEvents();

#if defined(Q_OS_IOS)
    setIosParams();
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    d.showMaximized();
#else
    d.show();
#endif
    app.exec();

    return 0;
}



