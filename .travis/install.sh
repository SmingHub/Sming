#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

pip install --user -r $TRAVIS_BUILD_DIR/samples/HttpServer_FirmwareUpload/requirements.txt

if [ "$TRAVIS_BUILD_STAGE_NAME" == "Test" ]; then
	sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 100
	pip3 install -r $TRAVIS_BUILD_DIR/docs/requirements.txt
fi

if [ "$SMING_ARCH" == "Esp8266" ]; then
	mkdir -p $TRAVIS_BUILD_DIR/opt/esp-alt-sdk
	
	if [ "$TRAVIS_OS_NAME" == "linux" ]; then
	  wget --no-verbose https://github.com/nodemcu/nodemcu-firmware/raw/2d958750b56fc60297f564b4ec303e47928b5927/tools/esp-open-sdk.tar.xz
	  tar -Jxvf esp-open-sdk.tar.xz; ln -s $(pwd)/esp-open-sdk/xtensa-lx106-elf $TRAVIS_BUILD_DIR/opt/esp-alt-sdk/.
	fi
	
fi # Esp8266
