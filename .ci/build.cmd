REM Windows build script

set CI_BUILD_DIR=%APPVEYOR_BUILD_FOLDER%

subst Z: %CI_BUILD_DIR%
set SMING_HOME=Z:\Sming

cd /d %SMING_HOME%
gcc -v

set MAKE_PARALLEL=make -j2

REM Move samples and tests into directory outside of the Sming repo.
set SMING_PROJECTS_DIR=%CI_BUILD_DIR%\projects
mkdir %SMING_PROJECTS_DIR%
move ..\samples %SMING_PROJECTS_DIR%
move ..\tests %SMING_PROJECTS_DIR%


set SMING_ARCH=Host
call :build

set SMING_ARCH=Esp8266
call :build

set SMING_ARCH=Esp32
call :build

goto :EOF


:build
call %SMING_HOME%\Arch\%SMING_ARCH%\Tools\ci\build.run.cmd || goto :error
goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
