REM Windows build script

SET SMING_HOME=%APPVEYOR_BUILD_FOLDER%\Sming

IF "%SMING_ARCH%" == "Esp8266" SET ESP_HOME=c:\Espressif

cd %SMING_HOME%
gcc -v

make help
make list-config

REM Compile the tools first
make tools V=1 || goto :error

cd %SMING_HOME%
make STRICT=1 || goto :error


if "%SMING_ARCH%" == "Host" (

	REM Build a couple of basic applications
	make Basic_Serial || goto :error
	make Basic_ProgMem || goto :error
	
	REM Run basic tests
	cd %SMING_HOME%\..\tests\HostTests
	make flash || goto :error

) ELSE (

	make Basic_Blink V=1 || goto :error
	make Basic_Ssl || goto :error
	make Basic_SmartConfig || goto :error

)

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
