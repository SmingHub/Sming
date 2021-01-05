#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo apt-get update

source "$SMING_HOME/Arch/Host/Tools/ci/install.sh"
source "$SMING_HOME/Arch/Esp8266/Tools/ci/install.sh"
source "$SMING_HOME/Arch/Esp32/Tools/ci/install.sh"
