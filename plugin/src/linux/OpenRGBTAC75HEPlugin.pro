QT += core gui widgets

TEMPLATE = lib
CONFIG += plugin c++17 link_pkgconfig
TARGET = OpenRGBTAC75HEPlugin

PKGCONFIG += hidapi-hidraw

HEADERS += \
    ../common/OpenRGBTAC75HEPlugin.h \
    ../common/RGBController_TAC75HE.h \
    ../common/TAC75HEHID.h \
    OpenRGB/OpenRGBPluginInterface.h \
    OpenRGB/RGBController/RGBControllerInterface.h

SOURCES += \
    ../common/OpenRGBTAC75HEPlugin.cpp \
    ../common/RGBController_TAC75HE.cpp \
    ../common/TAC75HEHID.cpp

OTHER_FILES += \
    ../common/OpenRGBTAC75HEPlugin.json

INCLUDEPATH += \
    ../common \
    OpenRGB \
    OpenRGB/RGBController \
    OpenRGB/dependencies/json

unix:!macx {
    # Link straight into the release directory.  build.sh creates it.
    DESTDIR = $$PWD/../../bin/linux

    isEmpty(PREFIX): PREFIX = /usr
    target.path = $$PREFIX/lib/openrgb/plugins/
    INSTALLS += target
}
