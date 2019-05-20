REM Appveyor Windows build script

SET SMING_HOME=%APPVEYOR_BUILD_FOLDER%\Sming
SET ESP_HOME=c:\Espressif
SET PATH=%PATH%;%ESP_HOME%/utils
cd %SMING_HOME%
gcc -v

make help
make list-config

REM Compile the tools first
make tools V=1 || goto :error

IF "%SDK_VERSION%" == "2.0.0" (
  REM Build Host Emulator and run basic tests
  set SMING_ARCH=Host
  cd %SMING_HOME%
  make STRICT=1 || goto :error
  make Basic_Serial || goto :error
  make Basic_ProgMem || goto :error
  cd %SMING_HOME%\..\samples\HostTests
  make flash || goto :error
)

REM Build library and test sample apps
set SMING_ARCH=Esp8266
cd %SMING_HOME%
make STRICT=1 || goto :error
cd %SMING_HOME%\..\samples\Basic_Blink
make V=1 || goto :error
cd %SMING_HOME%\..\samples\Basic_Ssl
make || goto :error
cd %SMING_HOME%\..\samples\Basic_SmartConfig
make || goto :error

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
