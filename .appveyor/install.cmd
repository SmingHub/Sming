REM Windows install script

rmdir /s /q c:\MinGW
curl -LO %SMINGTOOLS%/MinGW.7z
7z -oC:\ x MinGW.7z

goto :%SMING_ARCH%

:Esp8266

	REM Old toolchain
	set TOOLCHAIN=esp-udk-win32.7z
	curl -LO %SMINGTOOLS%/%TOOLCHAIN%
	7z -o%UDK_ROOT% x %TOOLCHAIN%

	REM New toolchain
	mkdir %EQT_ROOT%
	set TOOLCHAIN=x86_64-w64-mingw32.xtensa-lx106-elf-dd9f9a2.1569802152.zip
	curl -LO https://github.com/earlephilhower/esp-quick-toolchain/releases/download/3.0.0-gnu2/%TOOLCHAIN%
	7z -o%EQT_ROOT% x %TOOLCHAIN%

	goto :EOF


:Host

	goto :EOF

