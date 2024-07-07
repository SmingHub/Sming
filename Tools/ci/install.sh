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

INSTALL_OPTS=(fonts)
if [ "$SMING_ARCH" = "Host" ]; then
    INSTALL_OPTS+=(optional)
fi

# Ensure default path is writeable
sudo chown "$USER" /opt

"$CI_BUILD_DIR/Tools/install.sh" "$SMING_ARCH" "${INSTALL_OPTS[@]}"

fi

source "$CI_BUILD_DIR/Tools/export.sh"
python "$CI_BUILD_DIR/Tools/ci/clean-tools.py" clean --delete
