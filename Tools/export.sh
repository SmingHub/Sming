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
    _SMIG_BASE=$(realpath "$_SOURCEDIR/..")
    SMING_HOME=${_SMIG_BASE}"/Sming"
    export SMING_HOME
    echo "SMING_HOME: $SMING_HOME"
fi

# Common
export PYTHON=${PYTHON:=$(which python3)}

# Esp8266
export ESP_HOME=${ESP_HOME:=${_SMIG_BASE}/esp-quick-toolchain}

# Esp32
export IDF_PATH=${IDF_PATH:=${_SMIG_BASE}/esp-idf}
export IDF_TOOLS_PATH=${IDF_TOOLS_PATH:=${_SMIG_BASE}/esp32}

# Rp2040
export PICO_TOOLCHAIN_PATH=${PICO_TOOLCHAIN_PATH:=${_SMIG_BASE}/rp2040}

# Provide non-apple CLANG (e.g. for rbpf library)
if [ -n "$GITHUB_ACTIONS" ] && [ "$(uname)" = "Darwin" ]; then
CLANG="$(brew --prefix llvm@15)/bin/clang"
export CLANG
fi
