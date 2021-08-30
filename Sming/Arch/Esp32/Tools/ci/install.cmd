REM Esp32 install.cmd

if "%IDF_PATH%"=="" goto :EOF
if "%IDF_TOOLS_PATH%"=="" goto :EOF

git clone -b sming/release/v4.3 https://github.com/mikee47/esp-idf.git %IDF_PATH%

REM Install IDF tools and packages
python %IDF_PATH%\tools\idf_tools.py install
python -m pip install -r %IDF_PATH%\requirements.txt
