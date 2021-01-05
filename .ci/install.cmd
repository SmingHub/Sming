REM Windows install script

set CI_BUILD_DIR=%APPVEYOR_BUILD_FOLDER%
set SMING_HOME=%CI_BUILD_DIR%\Sming

SETX CI_BUILD_DIR %CI_BUILD_DIR%
SETX SMING_HOME %SMING_HOME%

call :install Host
call :install Esp8266
call :install Esp32

goto :EOF

:install
call %SMING_HOME%\Arch\%1\Tools\ci\install.cmd
goto :EOF
