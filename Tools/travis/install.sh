#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo apt-get clean
sudo apt-get update

# sudo apt-get remove -y libc6 libc6-dev --allow-remove-essential
# sudo apt-get autoremove

sudo apt-get install -y g++-9-multilib


if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi



