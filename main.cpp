#include <QtWidgets/qapplication.h>
#include <QtCore/QLoggingCategory>

#include "mainwindow.h"
#include "myNativeWrapperFunctions.h"

int main(int argc, char *argv[])
{
    Qt::ScreenOrientation ScreenMode;
    QApplication app(argc, argv);

    MainWindow d;

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



