/****************************************************************************
** Meta object code from reading C++ file 'ThumbnailPanel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/ThumbnailPanel.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ThumbnailPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN15ThumbnailWorkerE_t {};
} // unnamed namespace

template <> constexpr inline auto ThumbnailWorker::qt_create_metaobjectdata<qt_meta_tag_ZN15ThumbnailWorkerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ThumbnailWorker",
        "thumbnailReady",
        "",
        "generation",
        "pageIndex",
        "QImage",
        "image",
        "processFile",
        "filePath",
        "pageCount",
        "dpi"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'thumbnailReady'
        QtMocHelpers::SignalData<void(int, int, QImage)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { QMetaType::Int, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Slot 'processFile'
        QtMocHelpers::SlotData<void(const QString &, int, float, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 }, { QMetaType::Int, 9 }, { QMetaType::Float, 10 }, { QMetaType::Int, 3 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ThumbnailWorker, qt_meta_tag_ZN15ThumbnailWorkerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ThumbnailWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ThumbnailWorkerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ThumbnailWorkerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15ThumbnailWorkerE_t>.metaTypes,
    nullptr
} };

void ThumbnailWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ThumbnailWorker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->thumbnailReady((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QImage>>(_a[3]))); break;
        case 1: _t->processFile((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<float>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[4]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ThumbnailWorker::*)(int , int , QImage )>(_a, &ThumbnailWorker::thumbnailReady, 0))
            return;
    }
}

const QMetaObject *ThumbnailWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThumbnailWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ThumbnailWorkerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ThumbnailWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ThumbnailWorker::thumbnailReady(int _t1, int _t2, QImage _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}
namespace {
struct qt_meta_tag_ZN14ThumbnailPanelE_t {};
} // unnamed namespace

template <> constexpr inline auto ThumbnailPanel::qt_create_metaobjectdata<qt_meta_tag_ZN14ThumbnailPanelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ThumbnailPanel",
        "pageSelected",
        "",
        "pageIndex",
        "pagesReordered",
        "QList<int>",
        "orderedPageIndices",
        "rotateLeftRequested",
        "rotateRightRequested",
        "deleteRequested",
        "copyRequested",
        "pageIndices",
        "cutRequested",
        "pasteRequested",
        "insertAfterPageIndex",
        "onItemClicked",
        "QListWidgetItem*",
        "item",
        "onThumbnailReady",
        "generation",
        "QImage",
        "image",
        "onContextMenuRequested",
        "QPoint",
        "pos"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'pageSelected'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'pagesReordered'
        QtMocHelpers::SignalData<void(const QList<int> &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Signal 'rotateLeftRequested'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'rotateRightRequested'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'deleteRequested'
        QtMocHelpers::SignalData<void(int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'copyRequested'
        QtMocHelpers::SignalData<void(const QList<int> &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 11 },
        }}),
        // Signal 'cutRequested'
        QtMocHelpers::SignalData<void(const QList<int> &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 11 },
        }}),
        // Signal 'pasteRequested'
        QtMocHelpers::SignalData<void(int)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Slot 'onItemClicked'
        QtMocHelpers::SlotData<void(QListWidgetItem *)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'onThumbnailReady'
        QtMocHelpers::SlotData<void(int, int, QImage)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 19 }, { QMetaType::Int, 3 }, { 0x80000000 | 20, 21 },
        }}),
        // Slot 'onContextMenuRequested'
        QtMocHelpers::SlotData<void(const QPoint &)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ThumbnailPanel, qt_meta_tag_ZN14ThumbnailPanelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ThumbnailPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QListWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ThumbnailPanelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ThumbnailPanelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14ThumbnailPanelE_t>.metaTypes,
    nullptr
} };

void ThumbnailPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ThumbnailPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->pageSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->pagesReordered((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 2: _t->rotateLeftRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->rotateRightRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->deleteRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->copyRequested((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 6: _t->cutRequested((*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 7: _t->pasteRequested((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onItemClicked((*reinterpret_cast<std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 9: _t->onThumbnailReady((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QImage>>(_a[3]))); break;
        case 10: _t->onContextMenuRequested((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(int )>(_a, &ThumbnailPanel::pageSelected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(const QList<int> & )>(_a, &ThumbnailPanel::pagesReordered, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(int )>(_a, &ThumbnailPanel::rotateLeftRequested, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(int )>(_a, &ThumbnailPanel::rotateRightRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(int )>(_a, &ThumbnailPanel::deleteRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(const QList<int> & )>(_a, &ThumbnailPanel::copyRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(const QList<int> & )>(_a, &ThumbnailPanel::cutRequested, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ThumbnailPanel::*)(int )>(_a, &ThumbnailPanel::pasteRequested, 7))
            return;
    }
}

const QMetaObject *ThumbnailPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThumbnailPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ThumbnailPanelE_t>.strings))
        return static_cast<void*>(this);
    return QListWidget::qt_metacast(_clname);
}

int ThumbnailPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void ThumbnailPanel::pageSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ThumbnailPanel::pagesReordered(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ThumbnailPanel::rotateLeftRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ThumbnailPanel::rotateRightRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ThumbnailPanel::deleteRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void ThumbnailPanel::copyRequested(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ThumbnailPanel::cutRequested(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void ThumbnailPanel::pasteRequested(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}
QT_WARNING_POP
