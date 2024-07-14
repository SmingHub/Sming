@echo off
REM Install MinGW

set MINGW_PATH=C:\MinGW
set MINGW_SRC=MinGW-2021-04-12.7z

%MINGW_PATH%\bin\mingw-get --version >nul 2>&1
if not errorlevel 1 goto :mingw_installed

if exist "%MINGW_PATH%" (
    echo.
    echo ** Found existing installation at %MINGW_PATH%
    echo.
    echo Please rename or remove then try again.
    echo.
    exit /b 1
)

set SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0

echo.
echo.
echo ** Installing MinGW
echo.
curl -Lo %MINGW_SRC% %SMINGTOOLS%/%MINGW_SRC% || goto :EOF
7z -o%MINGW_PATH% x %MINGW_SRC% || goto :EOF
del %MINGW_SRC%
goto :update_path

:mingw_installed
echo MinGW found

:update_path
@powershell -NoProfile -ExecutionPolicy unrestricted %~dp0update-path.ps1 '%MINGW_PATH%\bin;%MINGW_PATH%\msys\1.0\bin'
