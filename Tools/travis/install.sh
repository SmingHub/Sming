#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

uname -a
cat /etc/apt/sources.list | grep ^[^#]
echo "======================================="
# sudo apt-get update 
# sudo apt-get -y upgrade
sudo apt-mark showhold


sudo apt-get remove -y gcc-multilib
sudo apt-get autoremove

# lib32gcc-9-dev
# lib32stdc++6
# libx32gcc-9-dev
# libx32stdc++6
sudo apt-get install -y lib32stdc++-9-dev libx32stdc++-9-dev
sudo apt-get install -y gcc-9-multilib
sudo apt-get install -y g++-9-multilib

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
	source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi
