#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# apt-rdepends -r libc6 libc6-dev

sudo apt-get remove libc6-prof
sudo apt-get autoremove
sudo apt-get clean
sudo apt-get install -y g++-9-multilib


if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi



