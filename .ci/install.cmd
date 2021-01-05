REM Windows install script

rmdir /s /q c:\MinGW
curl -Lo MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x MinGW.7z

REM
REM SMING_ARCH=Host
REM

REM
REM SMING_ARCH=Esp8266
REM


REM Old toolchain
set TOOLCHAIN=esp-udk-win32.7z
curl -LO %SMINGTOOLS%/%TOOLCHAIN%
7z -o%UDK_ROOT% x %TOOLCHAIN%


REM New toolchain
mkdir %EQT_ROOT%
set TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-e6a192b.201211.zip
curl -LO %SMINGTOOLS%/%TOOLCHAIN%
7z -o%EQT_ROOT% x %TOOLCHAIN%

REM
REM SMING_ARCH=Esp32
REM
