
# Esp8266 
$env:UDK_ROOT = Join-Path $env:CI_BUILD_DIR "opt/esp-alt-sdk"
$env:EQT_ROOT = Join-Path $env:CI_BUILD_DIR "opt/esp-quick-toolchain"
if ($env:BUILD_COMPILER -eq "udk") {
$env:ESP_HOME = $env:UDK_ROOT
} else {
$env:ESP_HOME = $env:EQT_ROOT
}

# Esp32
$env:IDF_PATH = Join-Path $env:CI_BUILD_DIR "opt/esp-idf"
$env:IDF_TOOLS_PATH = Join-Path $env:CI_BUILD_DIR "opt/tools/esp32"
$env:IDF_BRANCH = "sming/dev/v4.3"

# Rp2040
$env:PICO_TOOLCHAIN_PATH = Join-Path $env:CI_BUILD_DIR "opt/rp2040"

# General
$env:SMINGTOOLS = "https://github.com/SmingHub/SmingTools/releases/download/1.0"

if ($IsWindows) {
    $env:PATH = "C:\Python39;C:\Python39\Scripts;C:\MinGW\msys\1.0\bin;C:\MinGW\bin;" + $env:PATH
    $env:PYTHON = "C:\Python39\python"
    $env:ESP32_PYTHON_PATH = "C:\Python39"
}
