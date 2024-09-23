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

    *)
        TOOLS_MISSING=0
        COMMANDS=(dfu-util bison flex gperf)
        for COMMAND in "${COMMANDS[@]}"; do
            if ! [ -x $(command -v "${COMMAND}") ]; then
                TOOLS_MISSING=1
                echo "Install programm ${COMMAND}"
            fi
        done
        INCLUDES=("/usr/include/ffi.h" "/usr/include/ssl/ssl.h")
        for INCLUDE in "${INCLUDES[@]}"; do
            if ! [ -f "${INCLUDE}" ]; then
                TOOLS_MISSING=1
                echo "Install development package providing ${INCLUDE}"
            fi
        done
        if [ $TOOLS_MISSING != 0 ]; then
            echo "ABORTING"
            exit 1
        fi
        PACKAGES=()
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


# Install IDF tools and packages (unless CI has restored from cache)
if [ -n "$CI_BUILD_DIR" ] && [ -d "$IDF_TOOLS_PATH/tools" ]; then
    printf "\n\n** Skipping IDF tools installation: '%s/tools' exists\n\n" "$IDF_TOOLS_PATH"
else
    # Be specific about which tools we want to install for each IDF version
    IDF_TOOL_PACKAGES=$(tr '\n' ' ' < "$SMING_HOME/Arch/Esp32/Tools/idf_tools-${INSTALL_IDF_VER}.lst")
    echo "Install: $IDF_TOOL_PACKAGES"
    python3 "$IDF_PATH/tools/idf_tools.py" --non-interactive install $IDF_TOOL_PACKAGES
    if [ -n "$VIRTUAL_ENV" ]; then
        unset VIRTUAL_ENV
        unset VIRTUAL_ENV_PROMPT
        export PATH="${PATH/$VIRTUAL_ENV\/bin:/}"
    fi
    python3 "$IDF_PATH/tools/idf_tools.py" --non-interactive install-python-env

    if [ -z "$KEEP_DOWNLOADS" ]; then
        rm -rf "$IDF_TOOLS_PATH/dist"
    fi
fi

fi
