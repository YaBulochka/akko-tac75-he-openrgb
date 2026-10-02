QT += core gui widgets

TEMPLATE = lib
CONFIG += plugin c++17
TARGET = OpenRGBTAC75HEPlugin

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

DISTFILES += OpenRGBTAC75HEPlugin.json
OTHER_FILES += OpenRGBTAC75HEPlugin.json

INCLUDEPATH += \
    ../common \
    OpenRGB \
    OpenRGB/RGBController \
    OpenRGB/dependencies/json

DEPENDPATH += ../common

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS WIN32

    INCLUDEPATH += OpenRGB/dependencies/hidapi-win/include

    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L$$PWD/OpenRGB/dependencies/hidapi-win/x64 -lhidapi
    } else {
        LIBS += -L$$PWD/OpenRGB/dependencies/hidapi-win/x86 -lhidapi
    }

    LIBS += -lws2_32 -lole32

    DESTDIR = $$PWD/../../bin/windows
}

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += hidapi-hidraw
}
