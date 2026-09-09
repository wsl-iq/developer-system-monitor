@echo off
setlocal enabledelayedexpansion

echo
echo Starting Developer System Monitor
echo

REM Get the project root directory (parent of scripts folder)
set "PROJECT_ROOT=%~dp0.."
cd /d "%PROJECT_ROOT%"

echo Project root: %CD%
echo.

REM Add MinGW to PATH for DLL dependencies
where g++ >nul 2>&1
if %errorlevel% equ 0 (
    for /f "delims=" %%i in ('where g++') do (
        set "GXX_PATH=%%~dpi"
        set "PATH=!GXX_PATH!;!PATH!"
        echo Added MinGW to PATH: !GXX_PATH!
        goto :mingw_added
    )
)

REM Check common MinGW locations
if exist "C:\mingw64\bin" (
    set "PATH=C:\mingw64\bin;%PATH%"
    echo Added C:\mingw64\bin to PATH
    goto :mingw_added
)

if exist "C:\msys64\mingw64\bin" (
    set "PATH=C:\msys64\mingw64\bin;%PATH%"
    echo Added C:\msys64\mingw64\bin to PATH
    goto :mingw_added
)

:mingw_added

REM Check if virtual environment exists
if not exist "venv\Scripts\activate.bat" (
    echo Virtual environment not found.
    echo Please run scripts\build.bat first.
    pause
    exit /b 1
)

REM Activate virtual environment
call venv\Scripts\activate.bat

REM Check if DLL exists
if not exist "python\SystemMonitorNative.dll" (
    echo Warning: SystemMonitorNative.dll not found in python\
    
    REM Try to find it
    if exist "build\bin\SystemMonitorNative.dll" (
        echo Found in build\bin\ - copying...
        copy /Y "build\bin\SystemMonitorNative.dll" "python\" >nul
    ) else if exist "build\SystemMonitorNative.dll" (
        echo Found in build\ - copying...
        copy /Y "build\SystemMonitorNative.dll" "python\" >nul
    )
)

echo.
echo Starting FastAPI backend...
echo.

REM Run the application
cd python
python main.py

echo.
echo Application stopped.
pause