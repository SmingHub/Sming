REM Host build.setup.cmd

REM Build documentation
make -C %SMING_HOME% docs || goto :error

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
