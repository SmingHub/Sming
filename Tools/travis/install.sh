#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

apt-rdepends -r libc6 libc6-dev

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi



