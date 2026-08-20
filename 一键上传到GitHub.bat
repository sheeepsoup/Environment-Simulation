@echo off
title Upload project to GitHub

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0git-upload.ps1"
set "UPLOAD_EXIT_CODE=%ERRORLEVEL%"

echo.
if not "%UPLOAD_EXIT_CODE%"=="0" (
    echo Upload did not finish. Please review the error above.
) else (
    echo You can close this window now.
)
pause >nul
exit /b %UPLOAD_EXIT_CODE%
