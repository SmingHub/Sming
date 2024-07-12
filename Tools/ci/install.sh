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

"$SMING_HOME/../Tools/install.sh" "$SMING_ARCH" "${INSTALL_OPTS[@]}"

fi

# Clean up tools installation
source "$SMING_HOME/../Tools/export.sh"
python "$SMING_HOME/../Tools/ci/clean-tools.py" clean --delete
