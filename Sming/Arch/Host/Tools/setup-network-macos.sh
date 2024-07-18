#!/bin/bash
#
#  Helper script to setup tap0 networking interface under MacOS
#
# See https://github.com/ntop/n2n/issues/773
#

if [ ! -e /dev/tap0 ]; then
  srcdir="$SMING_HOME/Arch/Host/Tools/macos"
  sudo cp -r "$srcdir/tap.kext" /Library/Extensions
  sudo cp -r "$srcdir/net.tunnelblick.tap.plist" /Library/LaunchDaemons
fi
sudo kmutil load -p /Library/Extensions/tap.kext
