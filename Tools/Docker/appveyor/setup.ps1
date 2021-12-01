#!/usr/bin/env pwsh
#
# Called indirectly via appveyor build script
#

cd /tmp
$env:SMING_HOME = "$(pwd)/sming/Sming"
. sming/Tools/ci/setenv.ps1

if($IsWindows) {
    choco install -y python3 --version 3.9
    choco install -y cmake llvm
    sming/Tools/ci/install.cmd all doc
} else {
    $env:EXTRA_PACKAGES = "clang iproute2"
    sming/Tools/install.sh all doc fonts
}

Remove-Item sming, downloads -Force -Recurse
