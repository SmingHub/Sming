#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

apt-rdepends -r lib32stdc++-9-dev libx32stdc++-9-dev

if [ -f "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh" ]; then
source "$TRAVIS_BUILD_DIR/Sming/Arch/$SMING_ARCH/Tools/travis/install.sh"
fi



