#include <QtWidgets/qapplication.h>
#include <QtCore/QLoggingCategory>

#include "mainwindow.h"
/*
void KeepScreenOn()
{
    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if ( activity.isValid() )
    {
        QJniObject window= activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
        if (window.isValid()) {
            const int FLAG_KEEP_SCREEN_ON= 128;
            //window.callObjectMethod("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON); // Method 1
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
            qDebug() << "-------------------Always ON -------------";
        }
    }
}
*/
int main(int argc, char *argv[])
{
    Qt::ScreenOrientation ScreenMode;

    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QApplication app(argc, argv);

    QScreen *s = QGuiApplication::primaryScreen();
    if(s != NULL){
        /*
        qDebug() << "nativeOrientation: " << s->nativeOrientation();
        qDebug() << "orientation: " << s->orientation();
        qDebug() << (s->isLandscape(s->nativeOrientation()) ? "nativeOrientation lanscape" : "nativeOrientation not lanscape");
        qDebug() << (s->isLandscape(s->orientation()) ? "orientation lanscape" : "orientation not lanscape");
        qDebug() << (s->isPortrait(s->nativeOrientation()) ? "nativeOrientation portrait" : "nativeOrientation not portrait");
        qDebug() << (s->isPortrait(s->orientation()) ? "orientation portrait" : "orientation not portrait");
        ScreenMode = s->orientation();
        QSizeF x = s->physicalSize();
        float Ssize = sqrt(( x.rheight() * x.rheight() ) + ( x.rwidth()* x.rwidth())) / 25.4;
        qDebug() << "Screen Size: " << x.rheight() << x.rwidth() << Ssize;;
*/
     //   if (ScreenMode == Qt::InvertedPortraitOrientation) ScreenMode = Qt::PortraitOrientation;
    }

    MainWindow d;
//    QObject::connect(&d, &MainWindow::accepted, &app, &QApplication::quit);

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    d.showMaximized();
#else
    //    d.showMaximized();
    d.show();
#endif
   // KeepScreenOn();
    app.exec();

    return 0;
}



