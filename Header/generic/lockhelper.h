#ifndef LOCKHELPER_H
#define LOCKHELPER_H

#pragma once

#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>

class KeepAwakeHelper
{
public:
    KeepAwakeHelper();
    void EnableKeepAwakeHelper();
    virtual ~KeepAwakeHelper();

private:
    QJniObject m_wakeLock;
};

#endif // LOCKHELPER_H
