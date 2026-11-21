@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0capture_com5.ps1" %*
echo.
pause
