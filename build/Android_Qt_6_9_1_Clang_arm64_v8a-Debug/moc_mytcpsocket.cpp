/****************************************************************************
** Meta object code from reading C++ file 'mytcpsocket.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Header/generic/mytcpsocket.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mytcpsocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN18NoButtonMessageBoxE_t {};
} // unnamed namespace

template <> constexpr inline auto NoButtonMessageBox::qt_create_metaobjectdata<qt_meta_tag_ZN18NoButtonMessageBoxE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "NoButtonMessageBox"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NoButtonMessageBox, qt_meta_tag_ZN18NoButtonMessageBoxE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject NoButtonMessageBox::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18NoButtonMessageBoxE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18NoButtonMessageBoxE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18NoButtonMessageBoxE_t>.metaTypes,
    nullptr
} };

void NoButtonMessageBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NoButtonMessageBox *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *NoButtonMessageBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NoButtonMessageBox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18NoButtonMessageBoxE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int NoButtonMessageBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_tag_ZN11MyTcpSocketE_t {};
} // unnamed namespace

template <> constexpr inline auto MyTcpSocket::qt_create_metaobjectdata<qt_meta_tag_ZN11MyTcpSocketE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MyTcpSocket",
        "sendMessage",
        "",
        "message",
        "doTransponder",
        "doStart",
        "parseIMU",
        "parent",
        "uint32_t",
        "uiReg",
        "uint16_t[]",
        "sReg",
        "doRadar",
        "const char*",
        "data",
        "length"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sendMessage'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'doTransponder'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'doStart'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'parseIMU'
        QtMocHelpers::SlotData<void(void *, uint32_t, uint16_t)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::VoidStar, 7 }, { 0x80000000 | 8, 9 }, { 0x80000000 | 10, 11 },
        }}),
        // Slot 'doRadar'
        QtMocHelpers::SlotData<void(void *, const char *, uint32_t)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::VoidStar, 7 }, { 0x80000000 | 13, 14 }, { 0x80000000 | 8, 15 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MyTcpSocket, qt_meta_tag_ZN11MyTcpSocketE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MyTcpSocket::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MyTcpSocketE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MyTcpSocketE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11MyTcpSocketE_t>.metaTypes,
    nullptr
} };

void MyTcpSocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyTcpSocket *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sendMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->doTransponder(); break;
        case 2: _t->doStart(); break;
        case 3: _t->parseIMU((*reinterpret_cast< std::add_pointer_t<void*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<uint16_t[]>>(_a[3]))); break;
        case 4: _t->doRadar((*reinterpret_cast< std::add_pointer_t<void*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<const char*>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MyTcpSocket::*)(const QString & )>(_a, &MyTcpSocket::sendMessage, 0))
            return;
    }
}

const QMetaObject *MyTcpSocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyTcpSocket::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MyTcpSocketE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MyTcpSocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void MyTcpSocket::sendMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
