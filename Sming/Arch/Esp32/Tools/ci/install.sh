# Esp32 install.sh

sudo apt-get install -y git wget flex bison gperf ninja-build ccache libffi-dev libssl-dev dfu-util

git clone -b release/v4.1 https://github.com/espressif/esp-idf.git $IDF_PATH

# Espressif downloads very slow, fetch from SmingTools
mkdir -p $IDF_TOOLS_PATH
ESPTOOLS=esp32-tools-linux-4.1.zip
wget --no-verbose $SMINGTOOLS/$ESPTOOLS
unzip $ESPTOOLS -d $IDF_TOOLS_PATH/dist

python $IDF_PATH/tools/idf_tools.py install
python -m pip install -r $IDF_PATH/requirements.txt
