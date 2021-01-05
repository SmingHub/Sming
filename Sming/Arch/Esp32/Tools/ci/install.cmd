REM Esp32 install.cmd

set IDF_PATH=C:\tools\esp-idf
set IDF_TOOLS_PATH=C:\tools\esp32

SETX IDF_PATH %IDF_PATH%
SETX IDF_TOOLS_PATH %IDF_TOOLS_PATH%

git clone -b release/v4.1 https://github.com/espressif/esp-idf.git %IDF_PATH%

REM Espressif downloads very slow, fetch those from SmingTools
mkdir %IDF_TOOLS_PATH%
set ESPTOOLS=esp32-tools-windows-4.1.7z
curl -LO %SMINGTOOLS%/%ESPTOOLS%
7z -o%IDF_TOOLS_PATH%\dist x %ESPTOOLS% 

call %IDF_PATH%\install.bat

python -m pip install -r %IDF_PATH%\requirements.txt
