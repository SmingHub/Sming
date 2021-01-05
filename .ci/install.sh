#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

export SMING_HOME=$CI_BUILD_DIR/Sming

sudo apt-get update

install() {
	source $SMING_HOME/Arch/$1/Tools/ci/install.sh
}

install Host
install Esp8266
install Esp32
