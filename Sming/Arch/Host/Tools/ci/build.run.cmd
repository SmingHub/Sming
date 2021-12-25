REM Host build.run.cmd

REM Build a couple of basic applications
%MAKE_PARALLEL% Basic_Serial Basic_ProgMem Basic_IFS STRICT=1 || goto :error

REM Run basic tests
%MAKE_PARALLEL% tests || goto :error

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
