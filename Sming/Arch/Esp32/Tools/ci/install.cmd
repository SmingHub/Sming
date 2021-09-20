REM Esp32 install.cmd

if "%IDF_PATH%"=="" goto :EOF
if "%IDF_TOOLS_PATH%"=="" goto :EOF

if "%IDF_REPO%"=="" set IDF_REPO="https://github.com/mikee47/esp-idf.git"
if "%IDF_BRANCH%"=="" set IDF_BRANCH="sming/release/v4.3"

git clone -b %IDF_BRANCH% %IDF_REPO% %IDF_PATH%

REM Install IDF tools and packages
python %IDF_PATH%\tools\idf_tools.py install
python -m pip install -r %IDF_PATH%\requirements.txt
