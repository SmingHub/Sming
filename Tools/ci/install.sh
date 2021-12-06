#!/bin/bash
#
# CI installer script
#
# May also be used for library CI builds. The following variables must be set:
#
#   CI_BUILD_DIR
#   SMING_HOME
#
set -ex

if [ -z "$SMING_TOOLS_PREINSTALLED" ]; then

# appveyor-specific
export PYTHON=$HOME/venv3.9/bin/python
export ESP32_PYTHON_PATH=$HOME/venv3.9/bin
source "$HOME/venv3.9/bin/activate"

if [ "$BUILD_DOCS" = "true" ]; then
    INSTALL_OPTS="doc"
else
    INSTALL_OPTS="fonts"
fi

"$SMING_HOME/../Tools/install.sh" ${SMING_ARCH,,} $INSTALL_OPTS

fi
