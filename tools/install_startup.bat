@echo off
rem ==============================================================================
rem install_startup.bat — Register N'kai System Tray Daemon in Windows Startup
rem ==============================================================================
setlocal EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "REPO_ROOT=%%~fI"
set "STARTUP_DIR=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup"
set "VBS_FILE=%STARTUP_DIR%\NkaiTray.vbs"
set "TRAY_SCRIPT=%SCRIPT_DIR%nkai_tray.py"

if "%1"=="uninstall" goto do_uninstall
if "%1"=="status" goto do_status

:do_install
echo ==============================================================================
echo   🛰️ N'KAI SYSTEM TRAY DAEMON: STARTUP INSTALLER
echo ==============================================================================
echo [1/3] Resolving pythonw.exe path...
for /f "delims=" %%I in ('python -c "import sys, os; print(os.path.join(os.path.dirname(sys.executable), 'pythonw.exe'))"') do set "PYTHONW=%%I"

if not exist "%PYTHONW%" (
    echo [ERROR] pythonw.exe not found at: %PYTHONW%
    exit /b 1
)
echo       Found: %PYTHONW%

echo [2/3] Writing silent launcher to Windows Startup folder...
echo       Target: %VBS_FILE%

(
echo Set WshShell = CreateObject^("WScript.Shell"^)
echo WshShell.CurrentDirectory = "%REPO_ROOT%"
echo cmd = Chr^(34^) ^& "%PYTHONW%" ^& Chr^(34^) ^& " " ^& Chr^(34^) ^& "%TRAY_SCRIPT%" ^& Chr^(34^)
echo WshShell.Run cmd, 0, False
) > "%VBS_FILE%"

if not exist "%VBS_FILE%" (
    echo [ERROR] Failed to write %VBS_FILE%
    exit /b 1
)

echo [3/3] Launching N'kai System Tray Daemon now...
start "" wscript.exe "%VBS_FILE%"

echo ==============================================================================
echo   ✅ SUCCESS: N'kai Tray Daemon installed and registered for silent boot!
echo   📍 System Tray Icon will now appear automatically upon Windows login.
echo ==============================================================================
goto :eof

:do_uninstall
echo Removing %VBS_FILE% ...
if exist "%VBS_FILE%" (
    del "%VBS_FILE%"
    echo ✅ Removed startup entry.
) else (
    echo Startup entry was not installed.
)
goto :eof

:do_status
if exist "%VBS_FILE%" (
    echo ✅ N'kai Tray Daemon is INSTALLED in Windows Startup.
    type "%VBS_FILE%"
) else (
    echo ⚪ N'kai Tray Daemon is NOT installed in Windows Startup.
)
goto :eof
