#
# Set environment for CI build
#
# This script is sourced from the CI action script via powershell,
# so that environment variables are persisted.
#
# Note that we can't do this in a batch script (export.cmd or build.cmd) because
# PATH gets truncated to 4095 characters: It's typically rather longer than this.
# Nor can we source a batch script from powershell, so everything has to be here.
#

# Use same path for all platforms, so CI action script can just use "\opt"
$env:SMING_TOOLS_DIR = Join-Path (Split-Path -Path $env:CI_BUILD_DIR -Qualifier) "opt"

# Esp8266 
$env:ESP_HOME = Join-Path $env:SMING_TOOLS_DIR "esp-quick-toolchain"

# Esp32
$env:IDF_PATH = Join-Path $env:SMING_TOOLS_DIR "esp-idf"
$env:IDF_TOOLS_PATH = Join-Path $env:SMING_TOOLS_DIR "esp32"

# Rp2040
$env:PICO_TOOLCHAIN_PATH = Join-Path $env:SMING_TOOLS_DIR "rp2040"

# MinGW
$env:PATH = "C:\MinGW\msys\1.0\bin;C:\MinGW\bin;$env:PATH"
