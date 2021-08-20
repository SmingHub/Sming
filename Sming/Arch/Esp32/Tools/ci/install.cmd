REM Esp32 install.cmd

if "%IDF_PATH%"=="" goto :EOF
if "%IDF_TOOLS_PATH%"=="" goto :EOF

git clone -b release/v4.3 https://github.com/espressif/esp-idf.git %IDF_PATH%

REM Apply IDF patches
set IDF_PATCH=%~dp0..\idf.patch
pushd %IDF_PATH%
git apply -v --ignore-whitespace --whitespace=nowarn "%IDF_PATCH%"

REM Install IDF tools and packages
python tools\idf_tools.py install
python -m pip install -r requirements.txt

popd

