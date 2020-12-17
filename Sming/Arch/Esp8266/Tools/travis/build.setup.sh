#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

unset SPIFFY
unset ESPTOOL2
unset SDK_BASE

export PATH=$PATH:$ESP_HOME/xtensa-lx106-elf/bin:$ESP_HOME/utils/
