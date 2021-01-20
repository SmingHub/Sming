REM Windows install script

python -m pip install --upgrade pip

rmdir /s /q c:\MinGW
curl -Lo MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x MinGW.7z

call %SMING_HOME%\Arch\%SMING_ARCH%\Tools\ci\install.cmd
