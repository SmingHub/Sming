REM Host install.cmd

set PATH=C:\Python38;C:\MinGW\msys\1.0\bin;C:\MinGW\bin;%PATH%
set PYTHON=C:\Python38\python

rmdir /s /q c:\MinGW
curl -Lo MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x MinGW.7z
