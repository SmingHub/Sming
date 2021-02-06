# Host install.sh

# Required by deployment script
if [ -n "$APPVEYOR" ]; then
    sudo apt-get install -y \
        jq \
        xmlstarlet
fi
