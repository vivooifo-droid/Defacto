@echo off
REM Defacto Compiler Installer - Windows
REM This is the file people download and run directly
REM Usage: defacto-installer.exe

setlocal enabledelayedexpansion
cd /d "%TEMP%"

echo.
echo ====================================
echo  Defacto Compiler Installer
echo  https://github.com/vivooifo-droid
echo ====================================
echo.

REM Check admin
net session >nul 2>&1
if errorlevel 1 (
    echo ERROR: This requires administrator privileges
    echo Please right-click and "Run as administrator"
    pause
    exit /b 1
)

REM Check dependencies
echo 14 Checking dependencies...

where /q g++ >nul
if errorlevel 1 (
    echo   [X] g++ not found
    echo.
    echo ERROR: Install Visual Studio Community or MinGW-w64
    echo https://visualstudio.microsoft.com/vs/community/
    pause
    exit /b 1
)
echo   [OK] g++

where /q make >nul
if errorlevel 1 (
    echo   [X] make not found
    echo.
    echo ERROR: Install with: choco install make
    pause
    exit /b 1
)
echo   [OK] make

where /q git >nul
if errorlevel 1 (
    echo   [X] git not found
    echo.
    echo ERROR: Install git from https://git-scm.com/
    pause
    exit /b 1
)
echo   [OK] git

echo   [OK] All dependencies OK
echo.

REM Download
echo 18 Downloading Defacto...
set REPO_DIR=%TEMP%\defacto-installer-%RANDOM%
mkdir "%REPO_DIR%"
git clone --depth 1 https://github.com/vivooifo-droid/Defacto.git "%REPO_DIR%" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to download repository
    pause
    exit /b 1
)
echo   [OK] Downloaded
echo.

REM Build
echo 18 Building compiler...
cd /d "%REPO_DIR%\compiler"
make clean >nul 2>&1
make windows >nul 2>&1
if not exist "defacto.exe" (
    echo ERROR: Build failed
    pause
    exit /b 1
)
echo   [OK] Built
echo.

REM Install
echo 18 Installing...
set INSTALL_PATH=%ProgramFiles%\Defacto
if not exist "%INSTALL_PATH%" mkdir "%INSTALL_PATH%"
copy "defacto.exe" "%INSTALL_PATH%\" >nul

echo   [OK] Installed to %INSTALL_PATH%
echo.

REM Add to PATH
for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "CURRENT_PATH=%%B"
if not "!CURRENT_PATH!"=="" (
    if not "!CURRENT_PATH:%INSTALL_PATH%=!"=="!CURRENT_PATH!" goto :path_exists
)
setx PATH "%CURRENT_PATH%;%INSTALL_PATH%" >nul
echo   [OK] Added to PATH
:path_exists

REM Cleanup
cd /d "%TEMP%"
rmdir /s /q "%REPO_DIR%" 2>nul

echo.
echo ====================================
echo  [OK] Installation Complete!
echo ====================================
echo.
echo Usage:
echo   defacto -h
echo   defacto -terminal program.de
echo   defacto -kernel -o kernel.bin os.de
echo.
echo Don't forget NASM:
echo   choco install nasm
echo   https://www.nasm.us/
echo.
echo Restart terminal or press Win+X to apply PATH changes
echo.
pause
