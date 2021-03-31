REM Windows install script

set SMINGTOOLS=https://github.com/SmingHub/SmingTools/releases/download/1.0

python -m pip install --upgrade pip -r %SMING_HOME%\..\Tools\requirements.txt

rmdir /s /q c:\MinGW
curl -Lo MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x MinGW.7z

call %SMING_HOME%\Arch\%SMING_ARCH%\Tools\ci\install.cmd
