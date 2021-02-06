# Esp32 install.sh

if [ -n "$IDF_PATH" ] && [ -n "$IDF_TOOLS_PATH" ]; then

sudo apt-get install -y \
    bison \
    ccache \
    dfu-util \
    flex \
    gperf \
    libffi-dev \
    libssl-dev \
    ninja-build

git clone -b release/v4.1 https://github.com/espressif/esp-idf.git $IDF_PATH

# Espressif downloads very slow, fetch from SmingTools
mkdir -p $IDF_TOOLS_PATH
ESPTOOLS=esp32-tools-linux-4.1.zip
wget --no-verbose $SMINGTOOLS/$ESPTOOLS
unzip $ESPTOOLS -d $IDF_TOOLS_PATH/dist

python3 $IDF_PATH/tools/idf_tools.py install
python3 -m pip install -r $IDF_PATH/requirements.txt

if [ -z "$KEEP_DOWNLOADS" ]; then
    rm $ESPTOOLS
    rm -rf $IDF_TOOLS_PATH/dist
fi

fi
