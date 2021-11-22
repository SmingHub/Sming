#!/bin/bash
#
# Esp8266 install.sh

if [ -d "$ESP_HOME" ]; then
    printf "\n\n** Skipping Esp8266 tools installation: '$ESP_HOME' exists\n\n"
else
    TOOLCHAIN=x86_64-linux-gnu.xtensa-lx106-elf-e6a192b.201211.tar.gz
    $WGET "$SMINGTOOLS/$TOOLCHAIN" -O "$DOWNLOADS/$TOOLCHAIN"
    mkdir -p "$ESP_HOME"
    tar -zxf "$DOWNLOADS/$TOOLCHAIN" -C "$ESP_HOME" --totals
fi
