@echo off
setlocal enabledelayedexpansion

echo
echo Cleaning Developer System Monitor
echo

REM Get the project root directory (parent of scripts folder)
set "PROJECT_ROOT=%~dp0.."
cd /d "%PROJECT_ROOT%"

echo Project root: %CD%
echo.

REM Set counters
set "REMOVED_COUNT=0"
set "CLEANED_COUNT=0"

echo [1/8] Cleaning build artifacts...
echo.

REM Remove build directory
if exist "build" (
    echo   Removing build directory...
    rmdir /s /q "build" 2>nul
    if !errorlevel! equ 0 (
        set /a REMOVED_COUNT+=1
        echo   Done.
    ) else (
        echo   Warning: Could not remove build directory
    )
) else (
    echo   Build directory not found - skipping
)

REM Remove CMake artifacts
if exist "CMakeCache.txt" (
    del /q "CMakeCache.txt" 2>nul
    set /a CLEANED_COUNT+=1
)
if exist "CMakeFiles" (
    rmdir /s /q "CMakeFiles" 2>nul
    set /a REMOVED_COUNT+=1
)
if exist "cmake_install.cmake" (
    del /q "cmake_install.cmake" 2>nul
    set /a CLEANED_COUNT+=1
)
if exist "Makefile" (
    del /q "Makefile" 2>nul
    set /a CLEANED_COUNT+=1
)

echo.
echo [2/8] Cleaning Python virtual environment...
echo.

REM Remove virtual environment
if exist "venv" (
    echo   Removing virtual environment...
    rmdir /s /q "venv" 2>nul
    if !errorlevel! equ 0 (
        set /a REMOVED_COUNT+=1
        echo   Done.
    ) else (
        echo   Warning: Could not remove virtual environment
    )
) else (
    echo   Virtual environment not found - skipping
)

echo.
echo [3/8] Cleaning Python cache files...
echo.

REM Remove all __pycache__ directories
for /d /r "python" %%d in (__pycache__) do (
    if exist "%%d" (
        echo   Removing: %%d
        rmdir /s /q "%%d" 2>nul
        set /a REMOVED_COUNT+=1
    )
)

REM Remove .pyc files
for /r "python" %%f in (*.pyc) do (
    if exist "%%f" (
        del /q "%%f" 2>nul
        set /a CLEANED_COUNT+=1
    )
)

REM Remove .pyo files
for /r "python" %%f in (*.pyo) do (
    if exist "%%f" (
        del /q "%%f" 2>nul
        set /a CLEANED_COUNT+=1
    )
)

echo.
echo [4/8] Cleaning log files...
echo.

REM Remove log files
if exist "python\logs" (
    echo   Cleaning log files...
    for %%f in (python\logs\*.log) do (
        if exist "%%f" (
            del /q "%%f" 2>nul
            set /a CLEANED_COUNT+=1
        )
    )
    for %%f in (python\logs\*.log.*) do (
        if exist "%%f" (
            del /q "%%f" 2>nul
            set /a CLEANED_COUNT+=1
        )
    )
    echo   Done.
) else (
    echo   Log directory not found - skipping
)

echo.
echo [5/8] Cleaning database files...
echo.

REM Remove database files
if exist "data\monitor.db" (
    echo   Removing database...
    del /q "data\monitor.db" 2>nul
    set /a CLEANED_COUNT+=1
    echo   Done.
) else (
    echo   Database not found - skipping
)

REM Remove any other database files
if exist "data\*.db" (
    for %%f in (data\*.db) do (
        if exist "%%f" (
            del /q "%%f" 2>nul
            set /a CLEANED_COUNT+=1
        )
    )
)

echo.
echo [6/8] Cleaning DLL files...
echo.

REM Remove SystemMonitorNative.dll
if exist "python\SystemMonitorNative.dll" (
    echo   Removing: python\SystemMonitorNative.dll
    del /q "python\SystemMonitorNative.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

if exist "python\core\SystemMonitorNative.dll" (
    echo   Removing: python\core\SystemMonitorNative.dll
    del /q "python\core\SystemMonitorNative.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

REM Remove MinGW dependency DLLs - individual checks
if exist "python\libstdc++-6.dll" (
    echo   Removing: python\libstdc++-6.dll
    del /q "python\libstdc++-6.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

if exist "python\libgcc_s_seh-1.dll" (
    echo   Removing: python\libgcc_s_seh-1.dll
    del /q "python\libgcc_s_seh-1.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

if exist "python\libwinpthread-1.dll" (
    echo   Removing: python\libwinpthread-1.dll
    del /q "python\libwinpthread-1.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

if exist "python\core\libstdc++-6.dll" (
    echo   Removing: python\core\libstdc++-6.dll
    del /q "python\core\libstdc++-6.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

if exist "python\core\libgcc_s_seh-1.dll" (
    echo   Removing: python\core\libgcc_s_seh-1.dll
    del /q "python\core\libgcc_s_seh-1.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

if exist "python\core\libwinpthread-1.dll" (
    echo   Removing: python\core\libwinpthread-1.dll
    del /q "python\core\libwinpthread-1.dll" 2>nul
    set /a CLEANED_COUNT+=1
)

echo.
echo [7/8] Cleaning configuration and data files...
echo.

REM Remove configuration file
if exist "data\config.json" (
    echo   Removing: data\config.json
    del /q "data\config.json" 2>nul
    set /a CLEANED_COUNT+=1
)

REM Remove reports
if exist "data\reports" (
    echo   Removing reports...
    rmdir /s /q "data\reports" 2>nul
    set /a REMOVED_COUNT+=1
)

echo.
echo [8/8] Cleaning temporary files...
echo.

REM Remove .egg-info directories
for /d /r "python" %%d in (*.egg-info) do (
    if exist "%%d" (
        echo   Removing: %%d
        rmdir /s /q "%%d" 2>nul
        set /a REMOVED_COUNT+=1
    )
)

REM Remove .pytest_cache
for /d /r "python" %%d in (.pytest_cache) do (
    if exist "%%d" (
        echo   Removing: %%d
        rmdir /s /q "%%d" 2>nul
        set /a REMOVED_COUNT+=1
    )
)

echo.
echo
echo Clean Complete!
echo
echo.
echo Summary:
echo   Directories removed: !REMOVED_COUNT!
echo   Files deleted: !CLEANED_COUNT!
echo.
echo To rebuild the project, run: scripts\build.bat
echo.
pause