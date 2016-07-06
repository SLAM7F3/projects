/****************************************************************************
** Meta object code from reading C++ file 'WebClient.h'
**
** Created: Thu Mar 13 16:56:49 2008
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../WebClient.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WebClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_WebClient[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      22,   10,   10,   10, 0x08,
      62,   10,   10,   10, 0x08,
      78,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WebClient[] = {
    "WebClient\0\0readData()\0"
    "slotError(QAbstractSocket::SocketError)\0"
    "slotConnected()\0slotHostFound()\0"
};

const QMetaObject WebClient::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_WebClient,
      qt_meta_data_WebClient, 0 }
};

const QMetaObject *WebClient::metaObject() const
{
    return &staticMetaObject;
}

void *WebClient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WebClient))
	return static_cast<void*>(const_cast< WebClient*>(this));
    return QObject::qt_metacast(_clname);
}

int WebClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: readData(); break;
        case 1: slotError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 2: slotConnected(); break;
        case 3: slotHostFound(); break;
        }
        _id -= 4;
    }
    return _id;
}
