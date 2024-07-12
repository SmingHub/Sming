@echo off

REM
REM Sming installer bootstrap script for Windows
REM

echo.
echo Welcome to Sming bootstrap installer
echo.

REM Check we have administrative priviledge
net session >nul 2>&1
if errorlevel 1 (
    echo Opening an administrative command prompt to continue installation...
    powershell -Command "Start-Process cmd.exe -ArgumentList '/K %~f0' -Verb RunAs"
    goto :EOF
)

REM Sync working directory
cd /d %~dp0

REM See if choco is already installed
choco --version >nul 2>&1
if not errorlevel 1 goto :install_packages

REM Fetch the Chocolatey powershell installer script.
REM Note: Local scripts are blocked by default for 'security reasons'
REM Microsoft have kindly set up some hoops for us to jump through
REM (presumably making this 'secure'). OK, lets jump now.
REM Warning: Get the fire extinguisher, hoops may be on fire.
@powershell -NoProfile -ExecutionPolicy unrestricted -Command "iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))"
if errorlevel 1 goto :EOF

setlocal
SET PATH=%ALLUSERSPROFILE%\chocolatey\bin;%PATH%

:install_packages

REM Install standard packages
choco install -y git Python CMake Ninja 7zip ccache || goto :EOF

REM Fetch Sming
echo Fetch sming...
git clone https://github.com/SmingHub/Sming sming || goto :EOF
REM Fix ownership - we'd prefer to just clone as a regular user but that requires impersonation. Don't go there.
icacls sming /setowner %USERNAME% /T /C /L /Q

REM Install MinGW: Fine to leave this with admin priviledge as it should be considered read-only
call sming\tools\mingw-install.cmd
if errorlevel 1 goto :EOF

echo.
echo OK. Now open a regular (non-administrative) command prompt and run 'sming\tools\install.cmd' to install required toolchains
