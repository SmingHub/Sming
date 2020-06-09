#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo apt-get install -y git wget flex bison gperf  \
                cmake ninja-build ccache libffi-dev libssl-dev dfu-util \
                python3 python3-pip python3-setuptools
    sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
    python -m pip install --upgrade virtualenv==16.7.9
    mkdir -p $TRAVIS_BUILD_DIR/opt
    cd $TRAVIS_BUILD_DIR/opt
    git clone -b v4.0 --recursive https://github.com/espressif/esp-idf.git
    cd $TRAVIS_BUILD_DIR/opt/esp-idf
    ./install.sh
    source $TRAVIS_BUILD_DIR/opt/esp-idf/export.sh
    export IDF_PATH PATH
fi
