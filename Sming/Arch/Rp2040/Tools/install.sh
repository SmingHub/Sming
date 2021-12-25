#!/bin/bash
#
# Rp2040 install.sh

$PKG_INSTALL ninja-build

if [ -d "$PICO_TOOLCHAIN_PATH/arm-none-eabi" ]; then
    printf "\n\n** Skipping Rp2040 tools installation: '$PICO_TOOLCHAIN_PATH' exists\n\n"
else
    TOOLCHAIN_VERSION="10.3-2021.07"
    TOOLCHAIN_BASE_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm"
    TOOLCHAIN_NAME="gcc-arm-none-eabi-$TOOLCHAIN_VERSION"
    TOOLCHAIN_FILE="$TOOLCHAIN_NAME-x86_64-linux.tar.bz2"
    TOOLCHAIN_URL="$TOOLCHAIN_BASE_URL/$TOOLCHAIN_VERSION/$TOOLCHAIN_FILE"
    $WGET "$TOOLCHAIN_URL" -O "$DOWNLOADS/$TOOLCHAIN_FILE"
    mkdir -p "$PICO_TOOLCHAIN_PATH"
    tar -jxf "$DOWNLOADS/$TOOLCHAIN_FILE" -C "$PICO_TOOLCHAIN_PATH" --totals --transform='s|^/*||'
    mv "$PICO_TOOLCHAIN_PATH/$TOOLCHAIN_NAME/"* "$PICO_TOOLCHAIN_PATH"
fi
