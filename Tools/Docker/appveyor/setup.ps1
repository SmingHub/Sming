#!/usr/bin/env pwsh
#
# Called indirectly via appveyor build script
#

cd /tmp
$env:SMING_HOME = "$(pwd)/sming/Sming"
. sming/Tools/ci/setenv.ps1

if($IsWindows) {
    choco install -y --no-progress python3 --version 3.9
    choco install -y --no-progress cmake llvm
    sming/Tools/ci/install.cmd all doc
} else {
    $env:EXTRA_PACKAGES = "clang iproute2"
    sming/Tools/install.sh all doc fonts
}

pushd "$env:IDF_PATH"
git submodule update --init --recursive
popd

Remove-Item sming, downloads -Force -Recurse
