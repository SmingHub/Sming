@echo off

REM Check pre-requisites are installed and working, from whatever source

set MISSING_PACKAGES=
call :check_package python Python
call :check_package mingw-get MinGW
call :check_package cmake CMake
call :check_package ninja Ninja
where /q 7z
if errorlevel 1 set MISSING_PACKAGES=%MISSING_PACKAGES% 7zip
goto :EOF

REM Could use 'where' but Windows has a stub for python which isn't helpful
:check_package
%1 --version >nul 2>&1
if errorlevel 1 set MISSING_PACKAGES=%MISSING_PACKAGES% %2
goto :EOF
