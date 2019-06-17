REM Windows install script

goto :%SMING_ARCH%

:Esp8266

	IF "%SDK_VERSION%" == "1.5.0" (
		choco install esp8266-udk --source https://www.myget.org/F/kireevco-chocolatey/ -y --no-progress
		mkdir c:\Espressif\utils\ESP8266
		copy /b c:\Espressif\utils\memanalyzer.exe c:\Espressif\utils\ESP8266\memanalyzer.exe
	    copy /b c:\Espressif\utils\esptool.exe c:\Espressif\utils\ESP8266\esptool.exe
	) ELSE (
		choco install esp8266-udk --source https://www.myget.org/F/sming/ -y --no-progress
	)

	goto :EOF


:Host

	REM Ensure MinGW installation is up to date
	mingw-get update
	mingw-get upgrade

	goto :EOF

