# Esp8266 install.sh

export UDK_ROOT=$CI_BUILD_DIR/opt/esp-alt-sdk
export EQT_ROOT=$CI_BUILD_DIR/opt/esp-quick-toolchain

# Old toolchain
TOOLCHAIN=esp-open-sdk-linux-x86_64.tar.gz
wget --no-verbose $SMINGTOOLS/$TOOLCHAIN
tar -zxf $TOOLCHAIN
mkdir -p $CI_BUILD_DIR/opt/esp-alt-sdk
ln -s $(pwd)/esp-open-sdk/xtensa-lx106-elf $UDK_ROOT/.

# New toolchain
TOOLCHAIN=x86_64-linux-gnu.xtensa-lx106-elf-e6a192b.201211.tar.gz
wget --no-verbose $SMINGTOOLS/$TOOLCHAIN
mkdir -p $EQT_ROOT
tar -zxf $TOOLCHAIN -C $EQT_ROOT --totals
