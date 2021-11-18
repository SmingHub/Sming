REM
REM Windows CI install script
REM

echo.
echo.
echo ** Installing common python requirements
echo.
python -m pip install --upgrade pip -r %SMING_HOME%\..\Tools\requirements.txt

echo.
echo.
echo ** Installing MinGW
echo.
rmdir /s /q c:\MinGW
curl -Lo MinGW.7z %SMINGTOOLS%/MinGW-2020-10-19.7z
7z -oC:\ x MinGW.7z

if "%1" == "all" (
    call :install Host Esp8266 Esp32 Rp2040
    goto :EOF
)

:install
if "%1" == "" goto :EOF
echo.
echo.
echo ** Installing %1 toolchain
echo.
if "%1" == "doc" (
    call %SMING_HOME%\..\docs\Tools\install.cmd
) else (
    call %SMING_HOME%\Arch\%1\Tools\ci\install.cmd
)
shift
goto :install
