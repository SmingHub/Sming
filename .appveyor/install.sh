#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo apt-get update

ARCH_BASE=$APPVEYOR_BUILD_FOLDER/Sming/Arch
source "$ARCH_BASE/Host/Tools/appveyor/install.sh"
source "$ARCH_BASE/Esp8266/Tools/appveyor/install.sh"
source "$ARCH_BASE/Esp32/Tools/appveyor/install.sh"
