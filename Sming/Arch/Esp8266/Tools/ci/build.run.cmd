REM Esp8266 build.run.cmd

set ESP_HOME=%UDK_ROOT%
call :basic_build

%MAKE_PARALLEL% Basic_Ssl || goto :error
%MAKE_PARALLEL% Basic_SmartConfig || goto :error


set ESP_HOME=%EQT_ROOT%
make dist-clean
call :basic_build

%MAKE_PARALLEL% samples STRICT=1

goto :EOF


REM Build Basic_Blink sample and most of the framework Components
:basic_build

cd %SMING_PROJECTS_DIR%/samples/Basic_Blink
make help
make list-config
%MAKE_PARALLEL%
cd %SMING_HOME%

goto :EOF
