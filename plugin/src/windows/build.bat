@echo off
setlocal

if not exist "OpenRGBTAC75HEPlugin.pro" (
    echo Run this from plugin\src\windows
    exit /b 1
)

if not exist "OpenRGB\OpenRGBPluginInterface.h" (
    echo OpenRGB not found. Run setup-openrgb.bat first.
    exit /b 1
)

where qmake >nul 2>&1
if errorlevel 1 (
    echo qmake not found in PATH
    exit /b 1
)

echo qmake:
qmake -v
echo.

if exist build rmdir /s /q build
mkdir build
cd build

qmake ..\OpenRGBTAC75HEPlugin.pro CONFIG+=release
if errorlevel 1 exit /b 1

nmake
if errorlevel 1 (
    echo Build failed. Use "x64 Native Tools Command Prompt for VS 2022"
    exit /b 1
)

echo.
echo SUCCESS
echo DLL: plugin\bin\windows\OpenRGBTAC75HEPlugin.dll
