#!/bin/bash
#
# Esp8266 install.sh

EQT_REPO="https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.2.0-gcc10.3"
EQT_TOOLCHAIN="$(uname -m)-linux-gnu.xtensa-lx106-elf-c791b74.230224.tar.gz"

if [ -d "$ESP_HOME" ]; then
    printf "\n\n** Skipping Esp8266 tools installation: '$ESP_HOME' exists\n\n"
else
    $WGET "$EQT_REPO/$EQT_TOOLCHAIN" -O "$DOWNLOADS/$EQT_TOOLCHAIN"
    mkdir -p "$ESP_HOME"
    tar -zxf "$DOWNLOADS/$EQT_TOOLCHAIN" -C "$ESP_HOME" --totals
fi
