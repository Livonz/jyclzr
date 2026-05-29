@echo off
REM build_local.bat - Local build script for Windows (for reference)
REM Note: iOS builds require macOS. This script sets up the project only.

echo === JY Engine iOS Project Setup ===
echo.
echo WARNING: iOS builds require macOS with Xcode.
echo This script will only set up the project files.
echo.

REM Check for cmake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: cmake not found
    echo Install from: https://cmake.org/download/
    pause
    exit /b 1
)

echo Project files are ready.
echo.
echo To build for iOS:
echo 1. Transfer this project to a Mac
echo 2. Run: bash build_local.sh
echo.
echo Or push to GitHub and use the Actions workflow.
echo.
pause
