REM Esp8266 build.run.cmd

make -C "%SMING_PROJECTS_DIR%\samples\HttpServer_FirmwareUpload" python-requirements
REM %MAKE_PARALLEL% samples || goto :error

make clean samples-clean
%MAKE_PARALLEL% Basic_Blink ENABLE_CUSTOM_HEAP=1 DEBUG_VERBOSE_LEVEL=3 || goto :error
make HttpServer_ConfigNetwork ENABLE_CUSTOM_LWIP=2 STRICT=1 || goto :error
%MAKE_PARALLEL% Basic_Templates || goto :error

goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
