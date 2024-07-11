$TOOLS_DIR = "D:/opt"

# Esp8266 
$env:ESP_HOME = Join-Path $TOOLS_DIR "esp-quick-toolchain"

# Esp32
$env:IDF_PATH = Join-Path $TOOLS_DIR "esp-idf-$env:INSTALL_IDF_VER"
$env:IDF_TOOLS_PATH = Join-Path $TOOLS_DIR "esp32"

# Rp2040
$env:PICO_TOOLCHAIN_PATH = Join-Path $TOOLS_DIR "rp2040"

if (Test-Path "$env:PICO_TOOLCHAIN_PATH" ) {
    $env:SMING_TOOLS_PREINSTALLED = "true"
}

if ($IsWindows) {
    $env:PATH = "C:\MinGW\msys\1.0\bin;C:\MinGW\bin;$env:PATH"

    if ($null -eq $env:PYTHON_PATH) {
        $env:PYTHON_PATH = "C:\Python39-x64"
        if ( -not (Test-Path "$env:PYTHON_PATH") ) {
            $env:PYTHON_PATH = "C:\Python39"
        }
        if ( -not (Test-Path "$env:PYTHON_PATH") ) {
            $env:PYTHON_PATH = $(Get-Command python | Split-Path)
        }
    }

    $env:PATH = "$env:PYTHON_PATH;$env:PYTHON_PATH\Scripts;$env:PATH"
    $env:PYTHON = "$env:PYTHON_PATH\python"

    $env:PATH = "$env:PROGRAMFILES\CMake\bin;$env:PATH"

    $env:PATH = "$env:PROGRAMFILES\doxygen\bin;$env:PROGRAMFILES\Graphviz\bin;$env:PATH"

    if (Test-Path "$TOOLS_DIR" ) {
        $env:NINJA = (get-childitem -path "$TOOLS_DIR" -recurse -include "ninja.exe" | select-object -first 1).FullName
    }
}
