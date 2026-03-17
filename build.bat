@echo off
setlocal

:: ---------------------------------------------------------------------------
:: build.bat  —  Build PDFKill (Release)
:: Usage: build.bat [clean]
:: ---------------------------------------------------------------------------

set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%build
set VCPKG_TOOLCHAIN=C:\projects\meic\vcpkg\scripts\buildsystems\vcpkg.cmake
set QT_PREFIX=C:\Qt\6.10.2\msvc2022_64
set EXE=%BUILD_DIR%\Release\pdfkill.exe

:: ---------------------------
:: Optional clean
:: ---------------------------
if /I "%1"=="clean" (
    echo [build] Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /S /Q "%BUILD_DIR%"
    echo [build] Clean done.
)

:: ---------------------------
:: Kill running instance
:: ---------------------------
tasklist /FI "IMAGENAME eq pdfkill.exe" 2>NUL | find /I "pdfkill.exe" >NUL
if %ERRORLEVEL%==0 (
    echo [build] Stopping running pdfkill.exe...
    taskkill /F /IM pdfkill.exe >NUL 2>&1
    timeout /T 1 /NOBREAK >NUL
)

:: ---------------------------
:: CMake configure  (only if build dir doesn't exist or clean was requested)
:: ---------------------------
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo [build] Configuring...
    cmake -B "%BUILD_DIR%" ^
        -G "Visual Studio 17 2022" -A x64 ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_TOOLCHAIN%" ^
        -DCMAKE_PREFIX_PATH="%QT_PREFIX%"
    if errorlevel 1 (
        echo [build] ERROR: CMake configure failed.
        exit /B 1
    )
)

:: ---------------------------
:: Build
:: ---------------------------
echo [build] Building Release...
cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 (
    echo [build] ERROR: Build failed.
    exit /B 1
)

:: ---------------------------
:: Deploy Qt DLLs  (only when new exe or pdfium.dll missing)
:: ---------------------------
if not exist "%BUILD_DIR%\Release\pdfium.dll" (
    echo [build] Copying pdfium.dll...
    copy /Y "%PROJECT_DIR%third_party\pdfium\bin\pdfium.dll" "%BUILD_DIR%\Release\" >NUL
)

if not exist "%BUILD_DIR%\Release\Qt6Core.dll" (
    echo [build] Running windeployqt...
    "%QT_PREFIX%\bin\windeployqt.exe" "%EXE%"
)

echo.
echo [build] Done.  Executable: %EXE%
echo [build] Run with:  build\Release\pdfkill.exe

endlocal
