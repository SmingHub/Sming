REM Windows install script

goto :%SMING_ARCH%

:Esp8266

	choco install esp8266-udk --source https://www.myget.org/F/sming/ -y --no-progress

	goto :EOF


:Host

	REM Ensure MinGW installation is up to date
	mingw-get update
	mingw-get upgrade

	goto :EOF

