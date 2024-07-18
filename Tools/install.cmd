@echo off

REM
REM Windows install script
REM

if "%1"=="" (
    echo.
    echo Sming Installation options:
    echo   host      Host development tools
    echo   esp8266   ESP8266 development tools
    echo   esp32     ESP32 development tools
    echo   rp2040    RP2040 tools [Raspberry Pi Pico]
    echo   all       Install all architectures
    echo   doc       Tools required to build documentation
    echo.
    goto :EOF
)

call %~dp0check-packages.cmd
if "%MISSING_PACKAGES%" NEQ "" (
    echo Missing packages:%MISSING_PACKAGES%
    echo Please install manually or run `choco-install.cmd` from an administrative command prompt
    echo If already installed, please ensure they are in the system path
    echo See https://sming.readthedocs.io/en/latest/getting-started/windows/index.html
    goto :EOF
)

REM Leave file endings alone
git config --global --add core.autocrlf input

echo.
echo.
echo ** Installing common python requirements
echo.
python -m pip install --upgrade pip -r %~dp0requirements.txt

:install

REM Configure environment variables
call %~dp0export.cmd || goto :EOF

REM 
set DOWNLOADS=%SMING_TOOLS_DIR%\downloads
if not exist "%DOWNLOADS%" mkdir %DOWNLOADS%

if "%1" == "all" (
    call :install_loop Host Esp8266 Esp32 Rp2040
    goto :EOF
)

:install_loop
if "%1" == "" (
    echo.
    echo OK, install complete.
    echo.
    goto :EOF
)
if "%1" == "doc" (
    call %~dp0..\docs\Tools\install.cmd || goto :error
) else (
    call %SMING_HOME%\Arch\%1\Tools\install.cmd || goto :error
)
shift
goto :install_loop

:error
echo.
echo Installation failed
echo.
