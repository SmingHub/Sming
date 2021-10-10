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
    if [ "$(basename $SHELL)" = "zsh" ]; then
        _SOURCE=${(%):-%N}
    else
        _SOURCE=$BASH_SOURCE
    fi
    export SMING_HOME=$(readlink -m "$_SOURCE/../../Sming")
fi

# Common
export PYTHON=${PYTHON:=/usr/bin/python3}

# Esp8266
#export UDK_ROOT=${UDK_ROOT:=/opt/esp-open-sdk}
export EQT_ROOT=${EQT_ROOT:=/opt/esp-quick-toolchain}
export ESP_HOME=${ESP_HOME:=$EQT_ROOT}

# Esp32
export IDF_PATH=${IDF_PATH:=/opt/esp-idf}
export IDF_TOOLS_PATH=${IDF_TOOLS_PATH:=/opt/esp32}
export ESP32_PYTHON_PATH=${ESP32_PYTHON_PATH:=/usr/bin}
