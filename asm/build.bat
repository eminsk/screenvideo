@echo off
setlocal enabledelayedexpansion
title ScreenCapture Pro x64 - FASM Build System
color 0B

echo ===============================================================================
echo      ScreenCapture Pro v2.0 (x64 Native Assembly Edition)
echo      Compiling with Flat Assembler (FASM x64)
echo ===============================================================================
echo.

if not exist "FASM.EXE" (
    echo [ERROR] FASM.EXE not found in current directory!
    goto :error
)

echo [*] Assembling screenvideo.asm -^> screenvideo.exe ...
FASM.EXE screenvideo.asm screenvideo.exe
if errorlevel 1 goto :error

echo.
echo ===============================================================================
echo [SUCCESS] Build completed successfully!
echo Executable: %~dp0screenvideo.exe
for %%I in (screenvideo.exe) do (
    echo Binary Size: %%~zI bytes (~%%~zI / 1024 KB)
)
echo ===============================================================================
echo.

choice /C YN /M "Would you like to launch ScreenCapture Pro now?"
if errorlevel 2 goto :end
if errorlevel 1 start "" "%~dp0screenvideo.exe"

goto :end

:error
color 0C
echo.
echo ===============================================================================
echo [FAILED] Compilation aborted with errors!
echo ===============================================================================
pause
exit /b 1

:end
