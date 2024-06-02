REM Esp8266 install.cmd

set EQT_REPO=https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.2.0-gcc10.3
set EQT_TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-c791b74.230224.zip

mkdir %ESP_HOME%
curl -Lo %DOWNLOADS%/%EQT_TOOLCHAIN% %EQT_REPO%/%EQT_TOOLCHAIN%
7z -o%ESP_HOME% x %DOWNLOADS%/%EQT_TOOLCHAIN%
