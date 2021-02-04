#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Common install

sudo apt-get update

sudo update-alternatives --set gcc /usr/bin/gcc-9
python -m pip install --upgrade pip -r $SMING_HOME/../Tools/requirements.txt

sudo apt-get install -y gcc-9-multilib g++-9-multilib python3-setuptools

source $SMING_HOME/Arch/$SMING_ARCH/Tools/ci/install.sh
