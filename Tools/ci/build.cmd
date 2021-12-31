REM Windows build script

REM Don't leak this
set SMING_SECRET=

REM Build documentation job
if "%BUILD_DOCS%"=="true" (
    make -C %SMING_HOME%\..\docs html || goto :error
    goto :EOF
)

subst z: %CI_BUILD_DIR%
set SMING_HOME=z:\Sming

cd /d %SMING_HOME%
call Arch\%SMING_ARCH%\Tools\ci\build.setup.cmd || goto :error

env

set MAKE_PARALLEL=make -j%NUMBER_OF_PROCESSORS%

REM Move samples and tests into directory outside of the Sming repo.
set SMING_PROJECTS_DIR=%CI_BUILD_DIR%\projects
mkdir %SMING_PROJECTS_DIR%
move ..\samples %SMING_PROJECTS_DIR%
move ..\tests %SMING_PROJECTS_DIR%

REM Full compile checks please
set STRICT=1

REM Diagnostic info
cd /d %SMING_PROJECTS_DIR%/samples/Basic_Blink
make help
make list-config

REM HostTests must build for all architectures
%MAKE_PARALLEL% -C "%SMING_PROJECTS_DIR%/tests/HostTests" || goto :error

REM Start Arch-specific tests
cd /d %SMING_HOME%
call Arch\%SMING_ARCH%\Tools\ci\build.run.cmd || goto :error
goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
