if($IsWindows) {
    $TOOLS_DIR = "C:\tools"
} else {
    $TOOLS_DIR = "/opt"
}

# Esp8266 
$env:ESP_HOME = Join-Path $TOOLS_DIR "esp-quick-toolchain"

# Esp32
$env:IDF_PATH = Join-Path $TOOLS_DIR "esp-idf"
$env:IDF_TOOLS_PATH = Join-Path $TOOLS_DIR "esp32"
$env:IDF_BRANCH = "sming/release/v4.3"

# Rp2040
$env:PICO_TOOLCHAIN_PATH = Join-Path $TOOLS_DIR "rp2040"

# General
$env:SMINGTOOLS = "https://github.com/SmingHub/SmingTools/releases/download/1.0"

if ($IsWindows) {
    $env:PATH = "C:\Python39;C:\Python39\Scripts;C:\MinGW\msys\1.0\bin;C:\MinGW\bin;$env:PATH"
    $env:PYTHON = "C:\Python39\python"
    $env:ESP32_PYTHON_PATH = "C:\Python39"

    $env:PATH = "$env:PROGRAMFILES\CMake\bin;$env:PATH"

    if (Test-Path "$TOOLS_DIR" ) {
        $env:NINJA = (get-childitem -path "$TOOLS_DIR" -recurse -include "ninja.exe" | select-object -first 1).FullName
    }
}
