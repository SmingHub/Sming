# Host install.sh

# Required by deployment script
if [ -n "$APPVEYOR" ]; then
    $PKG_INSTALL \
        jq \
        xmlstarlet
fi
