REM Rp2040 install.cmd

if "%PICO_TOOLCHAIN_PATH%"=="" goto :undefined
if exist "%PICO_TOOLCHAIN_PATH%/arm-none-eabi" goto :installed

set TOOLCHAIN_VERSION=13.2.rel1
set TOOLCHAIN_BASE_URL=https://developer.arm.com/-/media/Files/downloads/gnu
set TOOLCHAIN_NAME=arm-gnu-toolchain-%TOOLCHAIN_VERSION%-mingw-w64-i686-arm-none-eabi
set TOOLCHAIN_FILE=%TOOLCHAIN_NAME%.zip
curl -Lo tmp.zip %TOOLCHAIN_BASE_URL%/%TOOLCHAIN_VERSION%/binrel/%TOOLCHAIN_FILE% || goto :EOF
7z -o"%PICO_TOOLCHAIN_PATH%-tmp" x tmp.zip || goto :EOF
del tmp.zip
move "%PICO_TOOLCHAIN_PATH%-tmp/%TOOLCHAIN_NAME%" "%PICO_TOOLCHAIN_PATH%"
rmdir "%PICO_TOOLCHAIN_PATH%-tmp"

goto :EOF


:undefined
echo.
echo ** Cannot install Rp2040 tools: PICO_TOOLCHAIN_PATH not defined
echo.
goto :EOF


:installed
echo.
echo ** Skipping Rp2040 tools installation: '%PICO_TOOLCHAIN_PATH%' exists
echo.
goto :EOF
