#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo dpkg --add-architecture i386
sudo apt update
sudo apt install software-properties-common
sudo aptitude install g++-9-multilib

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi
