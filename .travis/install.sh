#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

pip install --user -r $TRAVIS_BUILD_DIR/samples/HttpServer_FirmwareUpload/requirements.txt

if [ "$TRAVIS_BUILD_STAGE_NAME" == "Test" ]; then
	sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 100
	pip3 install -r $TRAVIS_BUILD_DIR/docs/requirements.txt
fi

if [ "$SMING_ARCH" == "Esp8266" ]; then
	if [ "$SDK_VERSION" == "1.5.0" ] && [ "$TRAVIS_OS_NAME" == "osx" ]; then
	  SDK_FILE_NAME="esp-alt-sdk-v${SDK_VERSION}.${SDK_BUILD}-macos-x86_64.zip"
	fi
	
	if [ "$SDK_VERSION" == "1.5.0" ] && [ "$TRAVIS_OS_NAME" == "linux" ]; then
	  SDK_FILE_NAME="esp-alt-sdk-v${SDK_VERSION}.${SDK_BUILD}-linux-x86_64.tar.gz"
	fi
	
	mkdir -p $TRAVIS_BUILD_DIR/opt/esp-alt-sdk
	
	if [ "$SDK_VERSION" == "1.5.0" ]; then
	  wget --no-verbose https://bintray.com/artifact/download/kireevco/generic/${SDK_FILE_NAME}
	  bsdtar -xf ${SDK_FILE_NAME} -C $TRAVIS_BUILD_DIR/opt/esp-alt-sdk
	fi
	
	if [ "$SDK_VERSION" != "1.5.0" ] && [ "$TRAVIS_OS_NAME" == "linux" ]; then
	  wget --no-verbose https://github.com/nodemcu/nodemcu-firmware/raw/2d958750b56fc60297f564b4ec303e47928b5927/tools/esp-open-sdk.tar.xz
	  tar -Jxvf esp-open-sdk.tar.xz; ln -s $(pwd)/esp-open-sdk/xtensa-lx106-elf $TRAVIS_BUILD_DIR/opt/esp-alt-sdk/.
	fi
	
	if [ "$SDK_VERSION" == "2.0.0" ] && [ "$TRAVIS_OS_NAME" == "linux" ]; then
	  wget --no-verbose https://www.espressif.com/sites/default/files/sdks/esp8266_nonos_sdk_v2.0.0_16_08_10.zip -O sdk.zip
	  unzip sdk.zip
	  ln -s $(pwd)/ESP8266_NONOS_SDK/ $TRAVIS_BUILD_DIR/opt/esp-alt-sdk/sdk
	fi
fi # Esp8266
