#!/bin/bash
#
# Set default environment variables from shell:
#
#       source export.sh
#
# Do this **before** calling install.sh
#
# These variables should be persisted.
# For example, with Ubuntu list these in ~/.pam_environment file:
#
#   SMING_HOME=/opt/sming/Sming
#   etc...
#
# With Windows WSL you can append them in .bashrc instead:
#
#   export SMING_HOME=/opt/sming/Sming
#   etc...
#

if [ -z "$SMING_HOME" ]; then
    if [ "$(basename "$SHELL")" = "zsh" ]; then
        _SOURCEDIR=$(dirname "${0:a}")
    else
        _SOURCEDIR=$(dirname "${BASH_SOURCE[0]}")
    fi
    SMING_HOME=$(realpath "$_SOURCEDIR/../Sming")
    export SMING_HOME
    echo "SMING_HOME: $SMING_HOME"
fi

# Common

# Python Virtual Environment
if [ -f ~/.venvs/Sming/bin/activate ]; then
	source ~/.venvs/Sming/bin/activate
fi

export PYTHON=${PYTHON:=$(which python3)}

# Esp8266
export ESP_HOME=${ESP_HOME:=/opt/esp-quick-toolchain}

# Esp32
export IDF_PATH=${IDF_PATH:=/opt/esp-idf}
export IDF_TOOLS_PATH=${IDF_TOOLS_PATH:=/opt/esp32}

# Rp2040
export PICO_TOOLCHAIN_PATH=${PICO_TOOLCHAIN_PATH:=/opt/rp2040}

# Provide non-apple CLANG (e.g. for rbpf library)
if [ -n "$GITHUB_ACTIONS" ] && [ "$(uname)" = "Darwin" ]; then
CLANG="$(brew --prefix llvm@15)/bin/clang"
export CLANG
fi
