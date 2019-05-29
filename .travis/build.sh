#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

env
unset SPIFFY
unset ESPTOOL2

export SMING_HOME=$TRAVIS_BUILD_DIR/Sming

# Check coding style
if [ "$TRAVIS_BUILD_STAGE_NAME" == "Test" ]; then
 	.travis/tools/clang/format-pr.sh;
fi

# Setup ARCH SDK paths
if [ "$SMING_ARCH" == "Esp8266" ]; then
	export ESP_HOME=$TRAVIS_BUILD_DIR/opt/esp-alt-sdk
	if [ "$SDK_VERSION" == "3.0.0" ]; then
		export SDK_BASE=$SMING_HOME/third-party/ESP8266_NONOS_SDK
	fi

	export PATH=$PATH:$ESP_HOME/xtensa-lx106-elf/bin:$ESP_HOME/utils/
fi

# Full compile checks please
export STRICT=1

# Diagnostic info
cd $SMING_HOME
make help
make list-config

# Build the framework
make

if [ "$TRAVIS_BUILD_STAGE_NAME" == "Test" ]; then
	make Basic_Blink Basic_DateTime Basic_Delegates Basic_Interrupts Basic_ProgMem Basic_Serial Basic_Servo LiveDebug DEBUG_VERBOSE_LEVEL=3
	cd ../tests/HostTests
	make flash
else
	make samples
	make clean samples-clean
	make ENABLE_CUSTOM_HEAP=1 STRICT=1
	make Basic_Blink ENABLE_CUSTOM_HEAP=1 DEBUG_VERBOSE_LEVEL=3
	
	make dist-clean
	make HttpServer_ConfigNetwork ENABLE_CUSTOM_LWIP=2 STRICT=1
fi
