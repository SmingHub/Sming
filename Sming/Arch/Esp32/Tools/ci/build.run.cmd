REM Esp32 build.run.cmd

%MAKE_PARALLEL% Basic_Blink Basic_WiFi HttpServer_ConfigNetwork DEBUG_VERBOSE_LEVEL=3 STRICT=1
%MAKE_PARALLEL% Basic_Ssl ENABLE_SSL=Bearssl DEBUG_VERBOSE_LEVEL=3 STRICT=1
