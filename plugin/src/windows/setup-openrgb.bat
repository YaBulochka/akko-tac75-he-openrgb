@echo off
setlocal

set "OPENRGB_TAG=release_1.1"

if not exist "OpenRGBTAC75HEPlugin.pro" (
    echo Run this from plugin\src\windows
    exit /b 1
)

if exist "OpenRGB\.git" (
    echo Updating OpenRGB %OPENRGB_TAG%...
    git -C OpenRGB fetch --depth 1 origin %OPENRGB_TAG%
    git -C OpenRGB reset --hard FETCH_HEAD
) else (
    echo Cloning OpenRGB %OPENRGB_TAG%...
    git clone --depth 1 --branch %OPENRGB_TAG% https://github.com/CalcProgrammer1/OpenRGB.git OpenRGB
)

if errorlevel 1 (
    echo Failed to get OpenRGB
    exit /b 1
)

echo OpenRGB commit:
git -C OpenRGB rev-parse --short HEAD
echo.
echo Done.
