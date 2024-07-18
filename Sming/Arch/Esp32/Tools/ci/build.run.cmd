REM Esp32 build.run.cmd

%MAKE_PARALLEL% Basic_Blink Basic_Ethernet Basic_WiFi HttpServer_ConfigNetwork DEBUG_VERBOSE_LEVEL=3 STRICT=1 || goto :error
%MAKE_PARALLEL% Basic_Ssl ENABLE_SSL=Axtls DEBUG_VERBOSE_LEVEL=3 STRICT=1 || goto :error

REM make sure that the Ota Library sample compiles for ESP32
%MAKE_PARALLEL% -C %SMING_HOME%\Libraries\OtaUpgradeMqtt\samples\Upgrade

goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
