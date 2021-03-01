REM Esp8266 install.cmd

call :install "%UDK_ROOT%" esp-udk-win32.7z
call :install "%EQT_ROOT%" x86_64-w64-mingw32.xtensa-lx106-elf-e6a192b.201211.zip
goto :EOF

:install
if "%1"=="" goto :EOF
mkdir %1
curl -LO %SMINGTOOLS%/%2
7z -o%1 x %2
