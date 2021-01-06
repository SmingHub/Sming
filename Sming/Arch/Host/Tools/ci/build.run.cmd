REM Host build.run.cmd

cd /d %SMING_PROJECTS_DIR%/samples/Basic_Blink
make help
make list-config
%MAKE_PARALLEL% || goto :error
cd /d %SMING_HOME%

REM Build a couple of basic applications
%MAKE_PARALLEL% Basic_Serial Basic_ProgMem STRICT=1 V=1 || goto :error

REM Run basic tests
%MAKE_PARALLEL% tests || goto :error

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
