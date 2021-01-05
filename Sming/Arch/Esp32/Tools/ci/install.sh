sudo apt-get install -y git wget flex bison gperf  \
            cmake ninja-build ccache libffi-dev libssl-dev dfu-util \
            python3 python3-pip python3-setuptools
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10

mkdir -p $CI_BUILD_DIR/opt
cd $CI_BUILD_DIR/opt
git clone -b release/v4.1 https://github.com/espressif/esp-idf.git

# Espressif downloads very slow, fetch those from SmingTools
mkdir -p $IDF_TOOLS_PATH
ESPTOOLS=esp32-tools-linux-4.1.zip
wget --no-verbose $SMINGTOOLS/$ESPTOOLS
unzip $ESPTOOLS -d $IDF_TOOLS_PATH/dist

$IDF_PATH/install.sh

python -m pip install -r $IDF_PATH/requirements.txt
