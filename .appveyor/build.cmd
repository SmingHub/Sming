REM Windows build script

REM Fix issue with "couldn't commit memory for cygwin heap" error (< 512 chars)
set path=c:\mingw\bin;c:\mingw\msys\1.0\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;^
C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Program Files\7-Zip;C:\Program Files (x86)\CMake\bin;^
C:\Python27;C:\Python27\Scripts;C:\Tools\curl\bin;C:\Program Files\PowerShell\6\;^
C:\Program Files\Git\cmd;C:\Program Files\Git\usr\bin

set SMING_HOME=%APPVEYOR_BUILD_FOLDER%\Sming

if "%build_compiler%" == "udk" set ESP_HOME=%UDK_ROOT%
if "%build_compiler%" == "eqt" set ESP_HOME=%EQT_ROOT%

cd %SMING_HOME%
gcc -v

set MAKE_PARALLEL=make -j2

REM Move samples and tests into directory outside of the Sming repo.
set SMING_PROJECTS_DIR=%APPVEYOR_BUILD_FOLDER%\..
move ../samples %SMING_PROJECTS_DIR%
move ../tests %SMING_PROJECTS_DIR%

REM This will build the Basic_Blink application and most of the framework Components
cd %SMING_PROJECTS_DIR%/samples/Basic_Blink
make help
make list-config
%MAKE_PARALLEL% || goto :error

cd %SMING_HOME%

if "%SMING_ARCH%" == "Host" (

	REM Build a couple of basic applications
	%MAKE_PARALLEL% Basic_Serial Basic_ProgMem STRICT=1 V=1 || goto :error

	REM Run basic tests
	%MAKE_PARALLEL% tests || goto :error

) else (

	%MAKE_PARALLEL% Basic_Ssl || goto :error
	%MAKE_PARALLEL% Basic_SmartConfig || goto :error

)

goto :EOF

:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
