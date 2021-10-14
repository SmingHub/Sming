REM Esp32 build.run.cmd

%MAKE_PARALLEL% Basic_Blink Basic_WiFi HttpServer_ConfigNetwork DEBUG_VERBOSE_LEVEL=3 STRICT=1 || goto :error
%MAKE_PARALLEL% Basic_Ssl ENABLE_SSL=Bearssl DEBUG_VERBOSE_LEVEL=3 STRICT=1 || goto :error

ESP32_PROJECTS="Basic_Blink Basic_Ethernet"

REM esp32s2
%MAKE_PARALLEL% ESP_VARIANT=esp32s2 %ESP32_PROJECTS% || goto :error

REM esp32c3
%MAKE_PARALLEL% ESP_VARIANT=esp32c3 %ESP32_PROJECTS% || goto :error

REM esp32s3
%MAKE_PARALLEL% ESP_VARIANT=esp32s3 %ESP32_PROJECTS% || goto :error

REM make sure that the Ota Library sample compiles for ESP32
%MAKE_PARALLEL% -C %SMING_HOME%\Libraries\OtaUpgradeMqtt\samples\Upgrade

goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
