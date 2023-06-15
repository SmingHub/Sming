#!/bin/bash
#
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

# If directory exists and isn't a symlink then rename it
if [ ! -L "$IDF_PATH" ] && [ -d "$IDF_PATH" ]; then
    echo MOVING OLD
    mv "$IDF_PATH" "$IDF_PATH-old"
fi

INSTALL_IDF_VER="${INSTALL_IDF_VER:=4.4}"
IDF_CLONE_PATH="$(readlink -m "$IDF_PATH/..")/esp-idf-${INSTALL_IDF_VER}"
IDF_REPO="${IDF_REPO:=https://github.com/mikee47/esp-idf.git}"
IDF_BRANCH="sming/release/v${INSTALL_IDF_VER}"

if [ -d "$IDF_CLONE_PATH" ]; then
    printf "\n\n** Skipping ESP-IDF clone: '$IDF_CLONE_PATH' exists\n\n"
else
    echo "git clone -b $IDF_BRANCH $IDF_REPO $IDF_CLONE_PATH"
    git clone -b "$IDF_BRANCH" "$IDF_REPO" "$IDF_CLONE_PATH"
fi

# Create link to clone
rm -f "$IDF_PATH"
ln -s "$IDF_CLONE_PATH" "$IDF_PATH"

# Install IDF tools and packages
python3 "$IDF_PATH/tools/idf_tools.py" --non-interactive install
python3 "$IDF_PATH/tools/idf_tools.py" --non-interactive install-python-env
IDF_REQUIREMENTS="$IDF_PATH/requirements.txt"
if [ ! -f "$IDF_REQUIREMENTS" ]; then
    IDF_REQUIREMENTS="$IDF_PATH/tools/requirements/requirements.core.txt"
fi
python3 -m pip install --no-input -r "$IDF_REQUIREMENTS"

if [ -z "$KEEP_DOWNLOADS" ]; then
    rm -rf "$IDF_TOOLS_PATH/dist"
fi

fi
