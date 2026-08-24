@echo off
setlocal

rem Usage:
rem   copy_shaders_to_x64.bat
rem   copy_shaders_to_x64.bat Debug
rem   copy_shaders_to_x64.bat Release

set "CONFIG=%~1"
if not defined CONFIG set "CONFIG=Debug"

set "SOURCE=%~dp0shader"
set "DEST=%~dp0..\x64\%CONFIG%\shader"

if not exist "%SOURCE%\" (
    echo [ERROR] Shader source folder not found:
    echo %SOURCE%
    pause
    exit /b 1
)

if not exist "%~dp0..\x64\%CONFIG%\" (
    echo [ERROR] Build output folder not found:
    echo %~dp0..\x64\%CONFIG%
    echo Build the %CONFIG% configuration first.
    pause
    exit /b 1
)

echo Copying shaders...
echo From: %SOURCE%
echo To:   %DEST%

robocopy "%SOURCE%" "%DEST%" /E /R:1 /W:1 /NFL /NDL /NJH /NJS

rem Robocopy exit codes 0-7 are success states. 8 or higher is failure.
if errorlevel 8 (
    echo [ERROR] Shader copy failed. Robocopy exit code: %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Shader folder is ready beside Vulkan.exe.
exit /b 0
