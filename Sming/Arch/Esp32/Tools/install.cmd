REM Esp32 install.cmd

if "%IDF_PATH%"=="" goto :EOF
if "%IDF_TOOLS_PATH%"=="" goto :EOF

if exist "%IDF_PATH%" goto :setup

if "%IDF_REPO%"=="" set IDF_REPO="https://github.com/mikee47/esp-idf.git"
if "%INSTALL_IDF_VER%"=="" set INSTALL_IDF_VER=5.2
set IDF_BRANCH="sming/release/v%INSTALL_IDF_VER%"

if "%CI_BUILD_DIR%" NEQ "" (
    set IDF_INSTALL_OPTIONS=--depth 1 --recurse-submodules --shallow-submodules
)
git clone -b %IDF_BRANCH% %IDF_REPO% %IDF_PATH% %IDF_INSTALL_OPTIONS%

:setup

REM Install IDF tools and packages
python "%IDF_PATH%\tools\idf_tools.py" --non-interactive install
python -m pip install %SMINGTOOLS%/gevent-1.5.0-cp39-cp39-win_amd64.whl
python3 "%IDF_PATH%\tools\idf_tools.py" --non-interactive install-python-env

if "%INSTALL_IDF_VER%" == "5.0" goto :install_python
if "%INSTALL_IDF_VER%" == "5.2" goto :install_python
goto :EOF

:install_python
python "%IDF_PATH%\tools\idf_tools.py" --non-interactive install-python-env
