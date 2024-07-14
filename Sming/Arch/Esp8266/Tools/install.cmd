REM Esp8266 install.cmd

if "%ESP_HOME%" == "" goto :undefined
if exist "%ESP_HOME%" goto :installed

echo.
echo ** Installing ESP8266 toolchain
echo.

set EQT_REPO=https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.2.0-gcc10.3
set EQT_TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-c791b74.230224.zip

mkdir %ESP_HOME%
curl -Lo %DOWNLOADS%/%EQT_TOOLCHAIN% %EQT_REPO%/%EQT_TOOLCHAIN% || goto :EOF
7z -o%ESP_HOME% x %DOWNLOADS%/%EQT_TOOLCHAIN% || goto :EOF

goto :EOF


:undefined
echo.
echo ** Cannot install Esp8266 tools: ESP_HOME not defined
echo.
goto :EOF


:installed
echo.
echo ** Skipping Esp8266 tools installation: '%ESP_HOME%' exists
echo.
goto :EOF
