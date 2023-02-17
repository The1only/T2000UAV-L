#include <QtWidgets/qapplication.h>
#include <QtCore/QLoggingCategory>

#include "mainwindow.h"

int main(int argc, char *argv[])
{

    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QApplication app(argc, argv);

    MainWindow d;
//    QObject::connect(&d, &MainWindow::accepted, &app, &QApplication::quit);

#ifdef Q_OS_ANDROID
    d.showMaximized();
#else
    d.show();
#endif

    app.exec();

    return 0;
}

