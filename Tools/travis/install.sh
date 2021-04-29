#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo dpkg --add-architecture i386
sudo apt-get update -qq
sudo apt-get install -y g++-multilib
sudo apt-get install -y linux-libc-dev:i386

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
	source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi
