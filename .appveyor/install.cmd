REM Windows install script

rmdir /s /q c:\MinGW
curl -LO https://github.com/Attachix/SmingTools/releases/download/1.0/MinGW.7z
7z -oC:\ x MinGW.7z

goto :%SMING_ARCH%

:Esp8266

	choco install esp8266-udk --source https://www.myget.org/F/sming/ -y --no-progress

	goto :EOF


:Host

	goto :EOF

