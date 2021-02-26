# Esp32 install.sh

if [ -n "$IDF_PATH" ] && [ -n "$IDF_TOOLS_PATH" ]; then

PACKAGES=(\
    bison \
    ccache \
    dfu-util \
    flex \
    gperf \
    ninja-build \
    )

case $DIST in
    debian)
        PACKAGES+=(\
            libffi-dev \
            libssl-dev \
            )
        ;;

    fedora)
        PACKAGES+=(\
            libffi-devel \
            )
        ;;
esac

$PKG_INSTALL ${PACKAGES[*]}

if [ -d "$IDF_PATH" ]; then
    printf "\n\n** Skipping ESP-IDF clone: '$IDF_PATH' exists\n\n"
else
    git clone -b release/v4.1 https://github.com/espressif/esp-idf.git $IDF_PATH
fi

# Espressif downloads very slow, fetch from SmingTools
mkdir -p $IDF_TOOLS_PATH
ESPTOOLS=esp32-tools-linux-4.1.zip
$WGET $SMINGTOOLS/$ESPTOOLS -O $DOWNLOADS/$ESPTOOLS
unzip $DOWNLOADS/$ESPTOOLS -d $IDF_TOOLS_PATH/dist

python3 $IDF_PATH/tools/idf_tools.py install
python3 -m pip install -r $IDF_PATH/requirements.txt

if [ -z "$KEEP_DOWNLOADS" ]; then
    rm -rf $IDF_TOOLS_PATH/dist
fi

fi
