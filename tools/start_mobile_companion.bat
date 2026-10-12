@echo off
python "%~dp0manage_mobile_companion.py" status
if %ERRORLEVEL% NEQ 0 (
    python "%~dp0manage_mobile_companion.py" start
)
