REM Esp8266 install.cmd

call :install "%ESP_HOME%" x86_64-w64-mingw32.xtensa-lx106-elf-e6a192b.201211.zip
goto :EOF

:install
mkdir %1
curl -LO %SMINGTOOLS%/%2
7z -o%1 x %2
