@echo off
setlocal enabledelayedexpansion

echo
echo Building Developer System Monitor
echo

REM Get the project root directory (parent of scripts folder)
set "PROJECT_ROOT=%~dp0.."
cd /d "%PROJECT_ROOT%"

echo Project root: %CD%
echo.

echo [1/3] Building C++ Core...
echo.

REM Find MinGW64
set "MINGW_FOUND=0"
set "MINGW_PATH="

where g++ >nul 2>&1
if %errorlevel% equ 0 (
    set "MINGW_FOUND=1"
    echo Found g++ in PATH
    goto :mingw_found
)

if exist "C:\msys64\mingw64\bin\g++.exe" (
    set "MINGW_PATH=C:\msys64\mingw64\bin"
    set "MINGW_FOUND=1"
    echo Found MinGW64 at C:\msys64\mingw64
    goto :mingw_found
)

if exist "C:\mingw64\bin\g++.exe" (
    set "MINGW_PATH=C:\mingw64\bin"
    set "MINGW_FOUND=1"
    echo Found MinGW64 at C:\mingw64
    goto :mingw_found
)

:mingw_found

if "!MINGW_FOUND!"=="0" (
    echo ERROR: MinGW64 not found!
    pause
    exit /b 1
)

if not "!MINGW_PATH!"=="" (
    set "PATH=!MINGW_PATH!;!PATH!"
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Clean CMake cache
if exist "CMakeCache.txt" del /q "CMakeCache.txt" 2>nul
if exist "CMakeFiles" rmdir /s /q "CMakeFiles" 2>nul

REM Configure
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo Failed to configure CMake
    cd ..
    pause
    exit /b 1
)

REM Build
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Failed to build C++ Core
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo [2/3] Setting up Python environment...
echo.

REM Create virtual environment
if not exist "venv" (
    echo Creating virtual environment...
    python -m venv venv
)

REM Activate virtual environment
call venv\Scripts\activate.bat 2>nul
if %errorlevel% neq 0 (
    echo Warning: Could not activate virtual environment
    echo Using system Python instead
)

REM Install dependencies
echo Installing Python dependencies...
python -m pip install --upgrade pip --quiet
python -m pip install -r requirements.txt --quiet
if %errorlevel% neq 0 (
    echo Warning: Some dependencies failed to install
)

echo.
echo [3/3] Copying DLL files...
echo.

REM Copy DLL to correct locations
set "DLL_COPIED=0"

REM Check common build output locations
if exist "build\bin\SystemMonitorNative.dll" (
    echo Copying from build\bin\
    copy /Y "build\bin\SystemMonitorNative.dll" "python\" >nul
    copy /Y "build\bin\SystemMonitorNative.dll" "python\core\" >nul
    set "DLL_COPIED=1"
) else if exist "build\SystemMonitorNative.dll" (
    echo Copying from build\
    copy /Y "build\SystemMonitorNative.dll" "python\" >nul
    copy /Y "build\SystemMonitorNative.dll" "python\core\" >nul
    set "DLL_COPIED=1"
) else if exist "build\lib\SystemMonitorNative.dll" (
    echo Copying from build\lib\
    copy /Y "build\lib\SystemMonitorNative.dll" "python\" >nul
    copy /Y "build\lib\SystemMonitorNative.dll" "python\core\" >nul
    set "DLL_COPIED=1"
)

if "!DLL_COPIED!"=="1" (
    echo DLL copied successfully to:
    echo   python\SystemMonitorNative.dll
    echo   python\core\SystemMonitorNative.dll
) else (
    echo WARNING: DLL not found in expected locations
    echo Searching...
    for /r "build" %%f in (SystemMonitorNative.dll) do (
        if not "%%~dpf"=="%%~dpf" (
            echo Found: %%f
            copy /Y "%%f" "python\" >nul 2>&1
            copy /Y "%%f" "python\core\" >nul 2>&1
            set "DLL_COPIED=1"
        )
    )
)

echo.
echo Testing native interface...
python python\test_native.py 2>&1
if %errorlevel% neq 0 (
    echo.
    echo NOTE: Native test failed but build was successful.
    echo This may be due to missing DLL dependencies.
    echo.
    echo Try running: python python\test_native.py
    echo to see detailed error messages.
)

echo.
echo
echo Build Complete!
echo
echo.
echo DLL Location: build\bin\SystemMonitorNative.dll
echo Python: venv\
echo.
echo To run: scripts\run.bat
echo.
pause