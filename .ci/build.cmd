REM Windows build script

set CI_BUILD_DIR=%APPVEYOR_BUILD_FOLDER%

set PATH=C:\MinGW\msys\1.0\bin;C:\MinGW\bin;%PATH%
set PYTHON=C:\Python38\python

subst z: %CI_BUILD_DIR%
set SMING_HOME=z:\Sming

cd /d %SMING_HOME%
env
gcc -v

set MAKE_PARALLEL=make -j2

REM Move samples and tests into directory outside of the Sming repo.
set SMING_PROJECTS_DIR=%CI_BUILD_DIR%\projects
mkdir %SMING_PROJECTS_DIR%
move ..\samples %SMING_PROJECTS_DIR%
move ..\tests %SMING_PROJECTS_DIR%


call :build Host
call :build Esp8266
call :build Esp32

goto :EOF


:build
set SMING_ARCH=%1
call %SMING_HOME%\Arch\%1\Tools\ci\build.run.cmd || goto :error
goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
