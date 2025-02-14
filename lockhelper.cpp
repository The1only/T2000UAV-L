#include <QDebug>
#include "jni.h"

#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>

#include "lockhelper.h"

KeepAwakeHelper::KeepAwakeHelper()
{    
    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    if ( activity.isValid() )
    {

        QJniObject window= activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
        if (window.isValid()) {
            const int FLAG_KEEP_SCREEN_ON= 128;
            window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
            qDebug() << "-------------------Always ON -------------";
        }

        QJniObject serviceName = QJniObject::getStaticObjectField<jstring>("android/content/Context","POWER_SERVICE");
        if ( serviceName.isValid() )
        {
            QJniObject powerMgr = activity.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",serviceName.object<jobject>());
            if ( powerMgr.isValid() )
            {
                jint levelAndFlags = QJniObject::getStaticField<jint>("android/os/PowerManager","SCREEN_DIM_WAKE_LOCK");

                QJniObject tag = QJniObject::fromString( "My Tag" );

                m_wakeLock = powerMgr.callObjectMethod("newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;", levelAndFlags,tag.object<jstring>());
            }
        }
    }

    if ( m_wakeLock.isValid() )
    {
        m_wakeLock.callMethod<void>("acquire", "()V");
        qDebug() << "Locked device, can't go to standby anymore";
    }
    else
    {
        assert( false );
    }
}

KeepAwakeHelper::~KeepAwakeHelper()
{
    if ( m_wakeLock.isValid() )
    {
        m_wakeLock.callMethod<void>("release", "()V");
        qDebug() << "Unlocked device, can now go to standby";
    }
}
