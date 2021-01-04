sudo apt-get install -y git wget flex bison gperf  \
            cmake ninja-build ccache libffi-dev libssl-dev dfu-util \
            python3 python3-pip python3-setuptools
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10
mkdir -p $APPVEYOR_BUILD_FOLDER/opt
cd $APPVEYOR_BUILD_FOLDER/opt
git clone -b v4.1 --recursive https://github.com/espressif/esp-idf.git
export IDF_PATH=$APPVEYOR_BUILD_FOLDER/opt/esp-idf
$IDF_PATH/install.sh

python -m pip install -r $IDF_PATH/requirements.txt
