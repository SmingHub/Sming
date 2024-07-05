REM
REM Windows install script
REM
REM GIT, Python3 and Chocolatey must be installed and in the system path
REM

set DOWNLOADS=downloads
mkdir %DOWNLOADS%

set SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0

REM Leave file endings alone
git config --global --add core.autocrlf input

echo.
echo.
echo ** Installing common python requirements
echo.
python -m pip install --upgrade pip -r %SMING_HOME%\..\Tools\requirements.txt

choco install ninja ccache

echo.
echo.
echo ** Installing MinGW
echo.
rmdir /s /q c:\MinGW
curl -Lo %DOWNLOADS%\MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x %DOWNLOADS%\MinGW.7z

:install
if "%1" == "" goto :EOF
if "%1" == "all" (
    call :install Host Esp8266 Esp32 Rp2040
) else (
    echo.
    echo.
    echo ** Installing %1 toolchain
    echo.
    if "%1" == "doc" (
        call %SMING_HOME%\..\docs\Tools\install.cmd
    ) else (
        call %SMING_HOME%\Arch\%1\Tools\install.cmd
    )
)
shift
goto :install
