REM Esp32 install.cmd

if "%IDF_PATH%"=="" goto :EOF
if "%IDF_TOOLS_PATH%"=="" goto :EOF

if "%IDF_REPO%"=="" set IDF_REPO="https://github.com/mikee47/esp-idf.git"
if "%INSTALL_IDF_VER%"=="" set INSTALL_IDF_VER=4.4
set IDF_BRANCH="sming/release/v%INSTALL_IDF_VER%"

git clone -b %IDF_BRANCH% %IDF_REPO% %IDF_PATH%

REM Install IDF tools and packages
python "%IDF_PATH%\tools\idf_tools.py" --non-interactive install
python -m pip install %SMINGTOOLS%/gevent-1.5.0-cp39-cp39-win_amd64.whl
set IDF_REQUIREMENTS="%IDF_PATH%\requirements.txt"
if not exist "%IDF_REQUIREMENTS%" set IDF_REQUIREMENTS="%IDF_PATH%\tools\requirements\requirements.core.txt"
python -m pip install -r "%IDF_REQUIREMENTS%"

if "%INSTALL_IDF_VER%" == "5.0" goto :install_python
if "%INSTALL_IDF_VER%" == "5.2" goto :install_python
goto :EOF

:install_python
python "%IDF_PATH%\tools\idf_tools.py" --non-interactive install-python-env
