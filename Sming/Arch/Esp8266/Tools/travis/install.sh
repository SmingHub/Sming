#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
	# Old toolchain
	TOOLCHAIN=esp-open-sdk-linux-x86_64.tar.gz
	wget --no-verbose $SMINGTOOLS/$TOOLCHAIN
  	tar -zxf $TOOLCHAIN
	mkdir -p $TRAVIS_BUILD_DIR/opt/esp-alt-sdk
	ln -s $(pwd)/esp-open-sdk/xtensa-lx106-elf $TRAVIS_BUILD_DIR/opt/esp-alt-sdk/.

	# New toolchain
	TOOLCHAIN=x86_64-linux-gnu.xtensa-lx106-elf-a5c9861.1575819473.tar.gz
	wget --no-verbose https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-gnu5/$TOOLCHAIN
	mkdir -p opt/esp-quick-toolchain
  	tar -zxf $TOOLCHAIN -C opt/esp-quick-toolchain --totals
fi
