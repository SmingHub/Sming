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

if [ "$BUILD_DOCS" = "true" ]; then
    INSTALL_OPTS="doc"
else
    INSTALL_OPTS="fonts optional"
fi

# Ensure default path is writeable
sudo chown "$USER" /opt

"$SMING_HOME/../Tools/install.sh" $SMING_ARCH $INSTALL_OPTS

fi
