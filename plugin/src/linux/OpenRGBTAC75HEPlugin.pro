QT += core gui widgets

TEMPLATE = lib
CONFIG += plugin c++17 link_pkgconfig
TARGET = OpenRGBTAC75HEPlugin

PKGCONFIG += hidapi-hidraw

HEADERS += \
    OpenRGBTAC75HEPlugin.h \
    RGBController_TAC75HE.h \
    TAC75HEHID.h \
    OpenRGB/OpenRGBPluginInterface.h \
    OpenRGB/RGBController/RGBControllerInterface.h

SOURCES += \
    OpenRGBTAC75HEPlugin.cpp \
    RGBController_TAC75HE.cpp \
    TAC75HEHID.cpp

OTHER_FILES += \
    OpenRGBTAC75HEPlugin.json

INCLUDEPATH += \
    OpenRGB \
    OpenRGB/RGBController \
    OpenRGB/dependencies/json

unix:!macx {
    target.path = $$PREFIX/lib/openrgb/plugins/
    INSTALLS += target
}
