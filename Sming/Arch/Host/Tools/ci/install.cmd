REM Host install.cmd

setx PATH=C:\MinGW\msys\1.0\bin;C:\MinGW\bin;%PATH%
setx PYTHON=C:\Python38\python

rmdir /s /q c:\MinGW
curl -Lo MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x MinGW.7z
