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
    git clone -b release/v4.3 https://github.com/espressif/esp-idf.git $IDF_PATH
fi

# Apply IDF patches
IDF_PATCH=$(dirname $BASH_SOURCE)/idf.patch
pushd $IDF_PATH
git apply -v --ignore-whitespace --whitespace=nowarn $IDF_PATCH

# Install IDF tools and packages
python3 tools/idf_tools.py install
python3 -m pip install -r $IDF_PATH/requirements.txt

popd

if [ -z "$KEEP_DOWNLOADS" ]; then
    rm -rf $IDF_TOOLS_PATH/dist
fi

fi
