REM Host build.run.cmd

cd %SMING_PROJECTS_DIR%/samples/Basic_Blink
make help
make list-config
%MAKE_PARALLEL%
cd %SMING_HOME%

REM Build a couple of basic applications
%MAKE_PARALLEL% Basic_Serial Basic_ProgMem STRICT=1 V=1

REM Run basic tests
%MAKE_PARALLEL% tests
