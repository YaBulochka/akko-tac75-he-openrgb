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
    OpenRGB/dependencies/hidapi-win/include

DEPENDPATH += ../common

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS WIN32

    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L$$PWD/OpenRGB/dependencies/hidapi-win/x64 -lhidapi
        HIDAPI_DLL = $$PWD/OpenRGB/dependencies/hidapi-win/x64/hidapi.dll
    } else {
        LIBS += -L$$PWD/OpenRGB/dependencies/hidapi-win/x86 -lhidapi
        HIDAPI_DLL = $$PWD/OpenRGB/dependencies/hidapi-win/x86/hidapi.dll
    }

    LIBS += -lws2_32 -lole32 -ldelayimp
    QMAKE_LFLAGS += /DELAYLOAD:hidapi.dll

    DESTDIR = $$PWD/../../bin/windows

    QMAKE_POST_LINK += $$quote(cmd /c copy /Y $$shell_path($$HIDAPI_DLL) $$shell_path($$DESTDIR) $$escape_expand(\n\t))
}
