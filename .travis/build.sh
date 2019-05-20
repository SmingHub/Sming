#!/bin/bash
set -e # exit with nonzero exit code if anything fails

env
unset SPIFFY
unset ESPTOOL2

export SMING_HOME=$TRAVIS_BUILD_DIR/Sming
export ESP_HOME=$TRAVIS_BUILD_DIR/opt/esp-alt-sdk
if [ "$SDK_VERSION" = "3.0.0" ]; then
  export SDK_BASE=$SMING_HOME/third-party/ESP8266_NONOS_SDK
fi

cd $SMING_HOME

export PATH=$PATH:$ESP_HOME/xtensa-lx106-elf/bin:$ESP_HOME/utils/:$SMING_HOME/../.travis/tools
if [ "$SDK_VERSION" = "2.0.0" ]; then
  ../.travis/tools/clang/format-pr.sh;
fi

if [ "$SDK_VERSION" = "3.0.0" ]; then
  export SMING_ARCH=Host
  export STRICT=1
  make help
  make list-config
  make
  make Basic_Blink Basic_DateTime Basic_Delegates Basic_Interrupts Basic_ProgMem Basic_Serial Basic_Servo LiveDebug DEBUG_VERBOSE_LEVEL=3
  cd ../samples/HostTests
  make flash
  cd $SMING_HOME
fi

export SMING_ARCH=Esp8266
make help
make list-config
make STRICT=1
make samples
make clean samples-clean
make ENABLE_CUSTOM_HEAP=1 STRICT=1
make Basic_Blink ENABLE_CUSTOM_HEAP=1 DEBUG_VERBOSE_LEVEL=3
make dist-clean; make HttpServer_ConfigNetwork ENABLE_CUSTOM_LWIP=2 STRICT=1
