#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo dpkg --add-architecture i386
sudo apt update
sudo apt install software-properties-common
sudo aptitude install -y libc6-dev=2.31-0ubuntu9.2 libc6=2.31-0ubuntu9.2 g++-9-multilib linux-libc-dev:i386

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
	source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi
