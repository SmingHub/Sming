REM Esp8266 install.cmd

REM Old toolchain
set UDK_ROOT=C:\Espressif
SETX UDK_ROOT %UDK_ROOT%
set TOOLCHAIN=esp-udk-win32.7z
curl -LO %SMINGTOOLS%/%TOOLCHAIN%
7z -o%UDK_ROOT% x %TOOLCHAIN%

REM New toolchain
set EQT_ROOT=C:\esp-quick-toolchain
SETX EQT_ROOT %EQT_ROOT%
mkdir %EQT_ROOT%
set TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-e6a192b.201211.zip
curl -LO %SMINGTOOLS%/%TOOLCHAIN%
7z -o%EQT_ROOT% x %TOOLCHAIN%
