QT += core gui widgets

TEMPLATE = lib
CONFIG += plugin c++17
TARGET = OpenRGBTAC75HEPlugin

HEADERS += \
    ../common/OpenRGBTAC75HEPlugin.h \
    ../common/RGBController_TAC75HE.h \
    ../common/TAC75HEHID.h

SOURCES += \
    ../common/OpenRGBTAC75HEPlugin.cpp \
    ../common/RGBController_TAC75HE.cpp \
    ../common/TAC75HEHID.cpp

DISTFILES += OpenRGBTAC75HEPlugin.json

INCLUDEPATH += \
    ../common \
    OpenRGB \
    OpenRGB/RGBController \
    OpenRGB/dependencies/json \
    OpenRGB/dependencies/hidapi-hotplug-win/include

DEPENDPATH += ../common

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS WIN32

    # OpenRGB ships hidapi-hotplug.dll next to OpenRGB.exe on Windows. Link
    # against the same DLL instead of requiring a separate hidapi.dll.
    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L"$$PWD/OpenRGB/dependencies/hidapi-hotplug-win/x64" -lhidapi-hotplug
        HIDAPI_DLL = $$PWD/OpenRGB/dependencies/hidapi-hotplug-win/x64/hidapi-hotplug.dll
    } else {
        LIBS += -L"$$PWD/OpenRGB/dependencies/hidapi-hotplug-win/x86" -lhidapi-hotplug
        HIDAPI_DLL = $$PWD/OpenRGB/dependencies/hidapi-hotplug-win/x86/hidapi-hotplug.dll
    }

    LIBS += -lws2_32 -lole32

    DESTDIR = $$PWD/../../bin/windows

    QMAKE_POST_LINK += cmd /c copy /Y \"$$shell_path($$HIDAPI_DLL)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
}
