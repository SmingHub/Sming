REM Rp2040 build.run.cmd

set RP2040_PROJECTS=Basic_Blink Basic_Serial Basic_Storage HttpServer_ConfigNetwork

%MAKE_PARALLEL% %RP2040_PROJECTS% DEBUG_VERBOSE_LEVEL=3 || goto :error

goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
