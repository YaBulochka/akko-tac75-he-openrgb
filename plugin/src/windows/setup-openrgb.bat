@echo off
setlocal

if not exist "OpenRGBTAC75HEPlugin.pro" (
    echo Run this from plugin\src\windows
    exit /b 1
)

if exist "OpenRGB\.git" (
    echo Updating OpenRGB...
    git -C OpenRGB fetch --depth 1 origin master
    git -C OpenRGB reset --hard origin/master
) else (
    echo Cloning OpenRGB...
    git clone --depth 1 https://github.com/CalcProgrammer1/OpenRGB.git OpenRGB
)

if errorlevel 1 (
    echo Failed to get OpenRGB
    exit /b 1
)

echo OpenRGB commit:
git -C OpenRGB rev-parse --short HEAD
echo.
echo Done.
