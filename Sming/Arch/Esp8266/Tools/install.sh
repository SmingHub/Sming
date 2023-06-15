#!/bin/bash
#
# Esp8266 install.sh

EQT_REPO=https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-newlib4.0.0-gnu20

if [ -d "$ESP_HOME" ]; then
    printf "\n\n** Skipping Esp8266 tools installation: '$ESP_HOME' exists\n\n"
else
    TOOLCHAIN="$(uname -m)-linux-gnu.xtensa-lx106-elf-e6a192b.201211.tar.gz"
    $WGET "$EQT_REPO/$TOOLCHAIN" -O "$DOWNLOADS/$TOOLCHAIN"
    mkdir -p "$ESP_HOME"
    tar -zxf "$DOWNLOADS/$TOOLCHAIN" -C "$ESP_HOME" --totals
fi
