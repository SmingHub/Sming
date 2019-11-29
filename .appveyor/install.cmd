REM Windows install script

rmdir /s /q c:\MinGW
curl -LO https://github.com/SmingHub/SmingTools/releases/download/1.0/MinGW.7z
7z -oC:\ x MinGW.7z

goto :%SMING_ARCH%

:Esp8266

	REM Old toolchain
	choco install esp8266-udk --source https://www.myget.org/F/sming/ -y --no-progress

	REM New toolchain
	mkdir %EQT_ROOT%
	set TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-dd9f9a2.1569802152.zip
	curl -LO https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-gnu2/%TOOLCHAIN%
	7z -o%EQT_ROOT% x %TOOLCHAIN%

	goto :EOF


:Host

	goto :EOF

