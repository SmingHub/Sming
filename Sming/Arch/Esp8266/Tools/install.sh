# Esp8266 install.sh

# Old toolchain
if [ -n "$UDK_ROOT" ]; then
    if [ -d "$EQT_ROOT" ]; then
        printf "\n\n** Skipping Esp8266 tools installation: '$EQT_ROOT' exists\n\n"
    else
        TOOLCHAIN=esp-open-sdk-linux-x86_64.tar.gz
        $WGET $SMINGTOOLS/$TOOLCHAIN -O $DOWNLOADS/$TOOLCHAIN
        tar -zxf $DOWNLOADS/$TOOLCHAIN
        mkdir -p $UDK_ROOT
        ln -s $(pwd)/esp-open-sdk/xtensa-lx106-elf $UDK_ROOT/.
    fi
fi

# New toolchain
if [ -n "$EQT_ROOT" ]; then
    if [ -d "$EQT_ROOT" ]; then
        printf "\n\n** Skipping Esp8266 tools installation: '$EQT_ROOT' exists\n\n"
    else
        TOOLCHAIN=x86_64-linux-gnu.xtensa-lx106-elf-e6a192b.201211.tar.gz
        $WGET $SMINGTOOLS/$TOOLCHAIN -O $DOWNLOADS/$TOOLCHAIN
        mkdir -p $EQT_ROOT
        tar -zxf $DOWNLOADS/$TOOLCHAIN -C $EQT_ROOT --totals
    fi
fi
