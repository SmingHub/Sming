@echo off

REM
REM Configure all Sming environment variables for this session
REM

if "%1"=="" (
    echo ** Setting values for current session ONLY
    set PERSIST_VARS=
) else if "%1"=="-persist" (
    echo ** Values will be persisted
    set PERSIST_VARS=1
) else (
    echo Configures Sming environment variables
    echo export [-persist]
    echo   --persist Optionally persist values in user registry
    echo.
    goto :EOF
)

REM Set SMING_HOME using this script location as starting point
set CUR_DIR=%CD%
cd %~dp0..\Sming
call :setvar SMING_HOME "%CD%"
cd %CUR_DIR%
set CUR_DIR=

REM Location for toolchains
if "%SMING_TOOLS_DIR%"=="" set SMING_TOOLS_DIR=C:\tools

REM Esp8266
call :setvar ESP_HOME "%SMING_TOOLS_DIR%\esp-quick-toolchain"

REM Esp32
call :setvar IDF_PATH "%SMING_TOOLS_DIR%\esp-idf"
call :setvar IDF_TOOLS_PATH "%SMING_TOOLS_DIR%\esp32"

REM Rp2040
call :setvar PICO_TOOLCHAIN_PATH "%SMING_TOOLS_DIR%\rp2040"

call %~dp0check-packages.cmd
if "%MISSING_PACKAGES%" NEQ "" (
    echo ** ERROR: Missing packages:%MISSING_PACKAGES%.
    echo Please run choco-install.cmd or install these manually.
)

goto :EOF

:setvar
setlocal enabledelayedexpansion
set CUR_VALUE=!%1!
if "%CUR_VALUE%" NEQ "" (
    if "%CUR_VALUE%" NEQ "%~2" (
        echo ** WARNING: Overriding existing %1 value "%CUR_VALUE%"
    )
)
endlocal
echo %1 = %~2
if "%PERSIST_VARS%"=="1" setx %1 %2
set "%1=%~2"
goto :EOF
