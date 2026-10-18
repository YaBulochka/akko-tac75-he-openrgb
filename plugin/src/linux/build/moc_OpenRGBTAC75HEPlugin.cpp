/****************************************************************************
** Meta object code from reading C++ file 'OpenRGBTAC75HEPlugin.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.12.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../OpenRGBTAC75HEPlugin.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OpenRGBTAC75HEPlugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.12.0. It"
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
struct qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t {};
} // unnamed namespace

template <> constexpr inline auto OpenRGBTAC75HEPlugin::qt_create_metaobjectdata<qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "OpenRGBTAC75HEPlugin"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<OpenRGBTAC75HEPlugin, qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject OpenRGBTAC75HEPlugin::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t>.metaTypes,
    nullptr
} };

void OpenRGBTAC75HEPlugin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<OpenRGBTAC75HEPlugin *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *OpenRGBTAC75HEPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpenRGBTAC75HEPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN20OpenRGBTAC75HEPluginE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "OpenRGBPluginInterface"))
        return static_cast< OpenRGBPluginInterface*>(this);
    if (!strcmp(_clname, "org.openrgb.OpenRGBPluginInterface"))
        return static_cast< OpenRGBPluginInterface*>(this);
    return QObject::qt_metacast(_clname);
}

int OpenRGBTAC75HEPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

#ifdef QT_MOC_EXPORT_PLUGIN_V2
static constexpr unsigned char qt_pluginMetaDataV2_OpenRGBTAC75HEPlugin[] = {
    0xbf, 
    // "IID"
    0x02,  0x78,  0x22,  'o',  'r',  'g',  '.',  'o', 
    'p',  'e',  'n',  'r',  'g',  'b',  '.',  'O', 
    'p',  'e',  'n',  'R',  'G',  'B',  'P',  'l', 
    'u',  'g',  'i',  'n',  'I',  'n',  't',  'e', 
    'r',  'f',  'a',  'c',  'e', 
    // "className"
    0x03,  0x74,  'O',  'p',  'e',  'n',  'R',  'G', 
    'B',  'T',  'A',  'C',  '7',  '5',  'H',  'E', 
    'P',  'l',  'u',  'g',  'i',  'n', 
    // "MetaData"
    0x04,  0xa9,  0x66,  'C',  'o',  'm',  'm',  'i', 
    't',  0x78,  0x29,  'O',  'p',  'e',  'n',  'R', 
    'G',  'B',  ' ',  'P',  'l',  'u',  'g',  'i', 
    'n',  ' ',  'A',  'P',  'I',  ' ',  '5',  ' ', 
    '/',  ' ',  'v',  'i',  'r',  't',  'u',  'a', 
    'l',  ' ',  'c',  'o',  'n',  't',  'r',  'o', 
    'l',  'l',  'e',  'r',  0x6b,  'D',  'e',  's', 
    'c',  'r',  'i',  'p',  't',  'i',  'o',  'n', 
    0x78,  0x29,  'D',  'i',  'r',  'e',  'c',  't', 
    ' ',  'M',  'o',  'd',  'e',  ' ',  's',  'u', 
    'p',  'p',  'o',  'r',  't',  ' ',  'f',  'o', 
    'r',  ' ',  't',  'h',  'e',  ' ',  'A',  'k', 
    'k',  'o',  ' ',  'T',  'A',  'C',  '7',  '5', 
    ' ',  'H',  'E',  0x62,  'I',  'd',  0x78,  0x19, 
    'o',  'r',  'g',  '.',  'o',  'p',  'e',  'n', 
    'r',  'g',  'b',  '.',  't',  'a',  'c',  '7', 
    '5',  'h',  'e',  'p',  'l',  'u',  'g',  'i', 
    'n',  0x64,  'N',  'a',  'm',  'e',  0x6d,  'A', 
    'k',  'k',  'o',  ' ',  'T',  'A',  'C',  '7', 
    '5',  ' ',  'H',  'E',  0x77,  'O',  'p',  'e', 
    'n',  'R',  'G',  'B',  'P',  'l',  'u',  'g', 
    'i',  'n',  'A',  'P',  'I',  'V',  'e',  'r', 
    's',  'i',  'o',  'n',  0x05,  0x63,  'U',  'r', 
    'l',  0x78,  0x33,  'h',  't',  't',  'p',  's', 
    ':',  '/',  '/',  'g',  'i',  't',  'h',  'u', 
    'b',  '.',  'c',  'o',  'm',  '/',  'Y',  'a', 
    'B',  'u',  'l',  'o',  'c',  'h',  'k',  'a', 
    '/',  'a',  'k',  'k',  'o',  '-',  't',  'a', 
    'c',  '7',  '5',  '-',  'h',  'e',  '-',  'o', 
    'p',  'e',  'n',  'r',  'g',  'b',  0x68,  'V', 
    'e',  'n',  'd',  'o',  'r',  'I',  'd',  0x72, 
    'e',  'c',  'h',  't',  'z',  'e',  'i',  't', 
    '-',  's',  'o',  'l',  'u',  't',  'i',  'o', 
    'n',  's',  0x67,  'V',  'e',  'r',  's',  'i', 
    'o',  'n',  0x65,  '1',  '.',  '1',  '.',  '0', 
    0x6a,  'V',  'e',  'r',  's',  'i',  'o',  'n', 
    'S',  't',  'r',  0x65,  '1',  '.',  '1',  '.', 
    '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN_V2(OpenRGBTAC75HEPlugin, OpenRGBTAC75HEPlugin, qt_pluginMetaDataV2_OpenRGBTAC75HEPlugin)
#else
QT_PLUGIN_METADATA_SECTION
Q_CONSTINIT static constexpr unsigned char qt_pluginMetaData_OpenRGBTAC75HEPlugin[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x22,  'o',  'r',  'g',  '.',  'o', 
    'p',  'e',  'n',  'r',  'g',  'b',  '.',  'O', 
    'p',  'e',  'n',  'R',  'G',  'B',  'P',  'l', 
    'u',  'g',  'i',  'n',  'I',  'n',  't',  'e', 
    'r',  'f',  'a',  'c',  'e', 
    // "className"
    0x03,  0x74,  'O',  'p',  'e',  'n',  'R',  'G', 
    'B',  'T',  'A',  'C',  '7',  '5',  'H',  'E', 
    'P',  'l',  'u',  'g',  'i',  'n', 
    // "MetaData"
    0x04,  0xa9,  0x66,  'C',  'o',  'm',  'm',  'i', 
    't',  0x78,  0x29,  'O',  'p',  'e',  'n',  'R', 
    'G',  'B',  ' ',  'P',  'l',  'u',  'g',  'i', 
    'n',  ' ',  'A',  'P',  'I',  ' ',  '5',  ' ', 
    '/',  ' ',  'v',  'i',  'r',  't',  'u',  'a', 
    'l',  ' ',  'c',  'o',  'n',  't',  'r',  'o', 
    'l',  'l',  'e',  'r',  0x6b,  'D',  'e',  's', 
    'c',  'r',  'i',  'p',  't',  'i',  'o',  'n', 
    0x78,  0x29,  'D',  'i',  'r',  'e',  'c',  't', 
    ' ',  'M',  'o',  'd',  'e',  ' ',  's',  'u', 
    'p',  'p',  'o',  'r',  't',  ' ',  'f',  'o', 
    'r',  ' ',  't',  'h',  'e',  ' ',  'A',  'k', 
    'k',  'o',  ' ',  'T',  'A',  'C',  '7',  '5', 
    ' ',  'H',  'E',  0x62,  'I',  'd',  0x78,  0x19, 
    'o',  'r',  'g',  '.',  'o',  'p',  'e',  'n', 
    'r',  'g',  'b',  '.',  't',  'a',  'c',  '7', 
    '5',  'h',  'e',  'p',  'l',  'u',  'g',  'i', 
    'n',  0x64,  'N',  'a',  'm',  'e',  0x6d,  'A', 
    'k',  'k',  'o',  ' ',  'T',  'A',  'C',  '7', 
    '5',  ' ',  'H',  'E',  0x77,  'O',  'p',  'e', 
    'n',  'R',  'G',  'B',  'P',  'l',  'u',  'g', 
    'i',  'n',  'A',  'P',  'I',  'V',  'e',  'r', 
    's',  'i',  'o',  'n',  0x05,  0x63,  'U',  'r', 
    'l',  0x78,  0x33,  'h',  't',  't',  'p',  's', 
    ':',  '/',  '/',  'g',  'i',  't',  'h',  'u', 
    'b',  '.',  'c',  'o',  'm',  '/',  'Y',  'a', 
    'B',  'u',  'l',  'o',  'c',  'h',  'k',  'a', 
    '/',  'a',  'k',  'k',  'o',  '-',  't',  'a', 
    'c',  '7',  '5',  '-',  'h',  'e',  '-',  'o', 
    'p',  'e',  'n',  'r',  'g',  'b',  0x68,  'V', 
    'e',  'n',  'd',  'o',  'r',  'I',  'd',  0x72, 
    'e',  'c',  'h',  't',  'z',  'e',  'i',  't', 
    '-',  's',  'o',  'l',  'u',  't',  'i',  'o', 
    'n',  's',  0x67,  'V',  'e',  'r',  's',  'i', 
    'o',  'n',  0x65,  '1',  '.',  '1',  '.',  '0', 
    0x6a,  'V',  'e',  'r',  's',  'i',  'o',  'n', 
    'S',  't',  'r',  0x65,  '1',  '.',  '1',  '.', 
    '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(OpenRGBTAC75HEPlugin, OpenRGBTAC75HEPlugin)
#endif  // QT_MOC_EXPORT_PLUGIN_V2

QT_WARNING_POP
