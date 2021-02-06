@echo off

REM
REM Sming installer bootstrap script for Windows
REM

REM Fetch the Chocolatey powershell installer script.
REM Note: Local scripts are blocked by default for 'security reasons'
REM Microsoft have kindly set up some hoops for us to jump through
REM (presumably making this 'secure'). OK, lets jump now.
REM Warning: Get the fire extinguisher, hoops may be on fire.
@powershell -NoProfile -ExecutionPolicy unrestricted -Command "iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))"
if "%errorlevel%" == "1" then goto :EOF

SET PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin

choco sources add -name smingrepo -source 'https://www.myget.org/F/sming/'

choco install -y sming
