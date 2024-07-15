REM Esp32 install.cmd

if "%IDF_PATH%"=="" goto :undefined
if "%IDF_TOOLS_PATH%"=="" goto :undefined

if "%INSTALL_IDF_VER%"=="" set INSTALL_IDF_VER=5.2

echo.
echo ** Installing Esp32 IDF version %INSTALL_IDF_VER%
echo.

for %%I in (%IDF_PATH%) do set IDF_CLONE_PATH=%%~dpIesp-idf-%INSTALL_IDF_VER%

if exist "%IDF_CLONE_PATH%" goto :cloned

if "%IDF_REPO%"=="" set IDF_REPO="https://github.com/mikee47/esp-idf.git"
set IDF_BRANCH="sming/release/v%INSTALL_IDF_VER%"

if "%CI_BUILD_DIR%" NEQ "" (
    set IDF_INSTALL_OPTIONS=--depth 1 --recurse-submodules --shallow-submodules
)
git clone -b %IDF_BRANCH% %IDF_REPO% %IDF_CLONE_PATH% %IDF_INSTALL_OPTIONS%
if errorlevel 1 goto :EOF

goto :setup


:undefined
echo.
echo ** Cannot install Esp32 tools: IDF_PATH or IDF_TOOLS_PATH not defined
echo.
goto :EOF


:cloned
echo.
echo ** Skipping ESP-IDF clone: '%IDF_CLONE_PATH%' exists
echo.
goto :setup


:setup

@echo Create link from "%IDF_PATH%" to "%IDF_CLONE_PATH%"
if exist "%IDF_PATH%" rmdir /q %IDF_PATH%
mklink /j %IDF_PATH% %IDF_CLONE_PATH%

REM Install IDF tools and packages (unless CI has restored from cache)
if "%CI_BUILD_DIR%" NEQ "" (
    if exist "%IDF_TOOLS_PATH%\tools" (
        echo Skipping IDF tools installation, "%IDF_TOOLS_PATH%\tools" exists
        goto :EOF
    )
)

REM Be specific about which tools we want to install for each IDF version
tr '\n' ' ' < "%~dp0idf_tools-%INSTALL_IDF_VER%.lst" > toolver.txt || goto :EOF
for /f "tokens=*" %%x in (toolver.txt) do set IDF_TOOL_PACKAGES=%%x
del toolver.txt
echo Install: %IDF_TOOL_PACKAGES%
python "%IDF_PATH%\tools\idf_tools.py" --non-interactive install %IDF_TOOL_PACKAGES% || goto :EOF
python "%IDF_PATH%\tools\idf_tools.py" --non-interactive install-python-env || goto :EOF

if "%CI_BUILD_DIR%" NEQ "" (
    del /q "%IDF_TOOLS_PATH%\dist\*"
)
