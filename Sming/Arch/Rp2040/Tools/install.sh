#!/bin/bash
#
# Rp2040 install.sh

$PKG_INSTALL ninja-build

if [ -d "$PICO_TOOLCHAIN_PATH/arm-none-eabi" ]; then
    printf "\n\n** Skipping Rp2040 tools installation: '$PICO_TOOLCHAIN_PATH' exists\n\n"
elif [ -n "$PICO_TOOLCHAIN_PATH" ]; then
    TOOLCHAIN_VERSION="13.2.rel1"
    TOOLCHAIN_BASE_URL="https://developer.arm.com/-/media/Files/downloads/gnu"
    TOOLCHAIN_FILE="arm-gnu-toolchain-$TOOLCHAIN_VERSION-$(uname -m)-arm-none-eabi.tar.xz"
    TOOLCHAIN_URL="$TOOLCHAIN_BASE_URL/$TOOLCHAIN_VERSION/binrel/$TOOLCHAIN_FILE"
    $WGET "$TOOLCHAIN_URL" -O "$DOWNLOADS/$TOOLCHAIN_FILE"
    mkdir -p "$PICO_TOOLCHAIN_PATH"
    tar -xf "$DOWNLOADS/$TOOLCHAIN_FILE" -C "$PICO_TOOLCHAIN_PATH" --totals --transform='s|^/*||'
    mv "$PICO_TOOLCHAIN_PATH/"*/* "$PICO_TOOLCHAIN_PATH"
fi
