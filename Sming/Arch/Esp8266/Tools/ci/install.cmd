REM Esp8266 install.cmd

REM Old toolchain
setx UDK_ROOT=C:\Espressif
set TOOLCHAIN=esp-udk-win32.7z
curl -LO %SMINGTOOLS%/%TOOLCHAIN%
7z -o%UDK_ROOT% x %TOOLCHAIN%

REM New toolchain
setx EQT_ROOT=C:\esp-quick-toolchain
mkdir %EQT_ROOT%
set TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-e6a192b.201211.zip
curl -LO %SMINGTOOLS%/%TOOLCHAIN%
7z -o%EQT_ROOT% x %TOOLCHAIN%
