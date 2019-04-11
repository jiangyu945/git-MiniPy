/****************************************************************************
** Meta object code from reading C++ file 'viewdlg.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "viewdlg.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'viewdlg.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ViewDlg_t {
    QByteArrayData data[8];
    char stringdata0[68];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ViewDlg_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ViewDlg_t qt_meta_stringdata_ViewDlg = {
    {
QT_MOC_LITERAL(0, 0, 7), // "ViewDlg"
QT_MOC_LITERAL(1, 8, 9), // "SigToSave"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 12), // "SigToMeasure"
QT_MOC_LITERAL(4, 32, 9), // "doViewImg"
QT_MOC_LITERAL(5, 42, 6), // "doSave"
QT_MOC_LITERAL(6, 49, 9), // "doMeasure"
QT_MOC_LITERAL(7, 59, 8) // "doCancel"

    },
    "ViewDlg\0SigToSave\0\0SigToMeasure\0"
    "doViewImg\0doSave\0doMeasure\0doCancel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ViewDlg[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   46,    2, 0x0a /* Public */,
       5,    0,   49,    2, 0x0a /* Public */,
       6,    0,   50,    2, 0x0a /* Public */,
       7,    0,   51,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QPixmap,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ViewDlg::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ViewDlg *_t = static_cast<ViewDlg *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->SigToSave(); break;
        case 1: _t->SigToMeasure(); break;
        case 2: _t->doViewImg((*reinterpret_cast< QPixmap(*)>(_a[1]))); break;
        case 3: _t->doSave(); break;
        case 4: _t->doMeasure(); break;
        case 5: _t->doCancel(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ViewDlg::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ViewDlg::SigToSave)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ViewDlg::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ViewDlg::SigToMeasure)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject ViewDlg::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ViewDlg.data,
      qt_meta_data_ViewDlg,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ViewDlg::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ViewDlg::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ViewDlg.stringdata0))
        return static_cast<void*>(const_cast< ViewDlg*>(this));
    return QDialog::qt_metacast(_clname);
}

int ViewDlg::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ViewDlg::SigToSave()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void ViewDlg::SigToMeasure()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
