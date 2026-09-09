@echo off
setlocal enabledelayedexpansion

echo 
echo Copying MinGW DLL dependencies
echo 

set "PROJECT_ROOT=%~dp0.."
cd /d "%PROJECT_ROOT%"

REM Find MinGW bin directory
set "MINGW_BIN="

where g++ >nul 2>&1
if %errorlevel% equ 0 (
    for /f "delims=" %%i in ('where g++') do (
        set "MINGW_BIN=%%~dpi"
        goto :found
    )
)

if exist "C:\mingw64\bin" (
    set "MINGW_BIN=C:\mingw64\bin"
    goto :found
)

if exist "C:\msys64\mingw64\bin" (
    set "MINGW_BIN=C:\msys64\mingw64\bin"
    goto :found
)

:found

if "!MINGW_BIN!"=="" (
    echo MinGW not found!
    pause
    exit /b 1
)

echo MinGW bin: !MINGW_BIN!
echo.

REM Copy required DLLs
set "DLLS_COPIED=0"

for %%f in (
    libstdc++-6.dll
    libgcc_s_seh-1.dll
    libwinpthread-1.dll
) do (
    if exist "!MINGW_BIN!\%%f" (
        copy /Y "!MINGW_BIN!\%%f" "python\" >nul
        copy /Y "!MINGW_BIN!\%%f" "python\core\" >nul
        echo Copied: %%f
        set /a DLLS_COPIED+=1
    ) else (
        echo Not found: %%f
    )
)

echo.
echo Copied !DLLS_COPIED! DLLs
echo.
pause