#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

uname -a
cat /etc/apt/sources.list | grep ^[^#]
echo "======================================="
sudo apt-get update 
sudo apt-get -y upgrade
sudo apt-mark showhold

dpkg -l | grep ^iU | awk '{print $2}' | xargs sudo dpkg --purge


sudo apt-get install gcc --only-upgrade
sudo apt-get install -y g++-9-multilib

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
	source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi
