/****************************************************************************
** Meta object code from reading C++ file 'DesEditor.h'
**
** Created: Wed Jul 25 10:08:50 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DesEditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DesEditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_Editor[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x08,
      20,    7,    7,    7, 0x08,
      32,    7,    7,    7, 0x08,
      43,    7,    7,    7, 0x08,
      51,    7,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Editor[] = {
    "Editor\0\0load_file()\0save_file()\0quit_app()\0about()\0aboutQt()\0"
};

const QMetaObject Editor::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_Editor,
      qt_meta_data_Editor, 0 }
};

const QMetaObject *Editor::metaObject() const
{
    return &staticMetaObject;
}

void *Editor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Editor))
	return static_cast<void*>(const_cast<Editor*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int Editor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: load_file(); break;
        case 1: save_file(); break;
        case 2: quit_app(); break;
        case 3: about(); break;
        case 4: aboutQt(); break;
        }
        _id -= 5;
    }
    return _id;
}
