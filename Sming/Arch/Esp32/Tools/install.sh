#!/bin/bash
#
# Esp32 install.sh

if [ -n "$IDF_PATH" ] && [ -n "$IDF_TOOLS_PATH" ]; then

PACKAGES=(\
    dfu-util
    )

case $DIST in
    debian)
        PACKAGES+=(\
            bison \
            flex \
            gperf \
            libffi-dev \
            libssl-dev \
            )
        ;;

    fedora)
        PACKAGES+=(\
            bison \
            flex \
            gperf \
            libffi-devel \
            )
        ;;

    darwin)
        ;;

esac

$PKG_INSTALL "${PACKAGES[@]}"

# If directory exists and isn't a symlink then rename it
if [ ! -L "$IDF_PATH" ] && [ -d "$IDF_PATH" ]; then
    echo MOVING OLD
    mv "$IDF_PATH" "$IDF_PATH-old"
fi

INSTALL_IDF_VER="${INSTALL_IDF_VER:=5.2}"
IDF_CLONE_PATH="$(dirname "$IDF_PATH")/esp-idf-${INSTALL_IDF_VER}"
IDF_REPO="${IDF_REPO:=https://github.com/mikee47/esp-idf.git}"
IDF_BRANCH="sming/release/v${INSTALL_IDF_VER}"

if [ -d "$IDF_CLONE_PATH" ]; then
    printf "\n\n** Skipping ESP-IDF clone: '%s' exists\n\n" "$IDF_CLONE_PATH"
else
    if [ -n "$CI_BUILD_DIR" ]; then
        IDF_INSTALL_OPTIONS="--depth 1 --recurse-submodules --shallow-submodules"
    fi
    echo "git clone -b $IDF_BRANCH $IDF_REPO $IDF_CLONE_PATH"
    git clone -b "$IDF_BRANCH" "$IDF_REPO" "$IDF_CLONE_PATH" $IDF_INSTALL_OPTIONS
fi

# Create link to clone
rm -f "$IDF_PATH"
ln -s "$IDF_CLONE_PATH" "$IDF_PATH"


# Skip installation for CI if already present
if [ -z "$CI_BUILD_DIR" ] || [ ! -d "$IDF_TOOLS_PATH/tools" ]; then

# Install IDF tools and packages
python3 "$IDF_PATH/tools/idf_tools.py" --non-interactive install
if [ -n "$VIRTUAL_ENV" ]; then
    unset VIRTUAL_ENV
    unset VIRTUAL_ENV_PROMPT
    export PATH="${PATH/$VIRTUAL_ENV\/bin:/}"
fi
python3 "$IDF_PATH/tools/idf_tools.py" --non-interactive install-python-env

if [ -z "$KEEP_DOWNLOADS" ]; then
    rm -rf "$IDF_TOOLS_PATH/dist"
fi

fi # CI install

fi
