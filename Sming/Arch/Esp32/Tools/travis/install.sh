#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    sudo apt-get install -y git wget flex bison gperf  \
                cmake ninja-build ccache libffi-dev libssl-dev dfu-util \
                python3 python3-pip python3-setuptools
    sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
    mkdir -p $TRAVIS_BUILD_DIR/opt
    cd $TRAVIS_BUILD_DIR/opt
    git clone -b v4.1 --recursive https://github.com/espressif/esp-idf.git
    export IDF_PATH=$TRAVIS_BUILD_DIR/opt/esp-idf
    $IDF_PATH/install.sh

    python -m pip install -r $IDF_PATH/requirements.txt
fi
