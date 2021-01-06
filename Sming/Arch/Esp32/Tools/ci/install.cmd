REM Esp32 install.cmd

git clone -b release/v4.1 https://github.com/espressif/esp-idf.git %IDF_PATH%

REM Espressif downloads very slow, fetch from SmingTools
mkdir %IDF_TOOLS_PATH%
set ESPTOOLS=esp32-tools-windows-4.1.7z
curl -LO %SMINGTOOLS%/%ESPTOOLS%
7z -o%IDF_TOOLS_PATH%\dist x %ESPTOOLS% 

python %IDF_PATH%\tools\idf_tools.py install
python -m pip install -r %IDF_PATH%\requirements.txt
