/**
 * @file main.cpp
 * @brief Implementation of main.
 *
 * Contains the implementation details for the main.
 */

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

    MainWindow d;

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    d.showMaximized();
#else
    d.show();
#endif
    app.exec();

    return 0;
}



