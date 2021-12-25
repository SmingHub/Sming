REM Rp2040 install.cmd

if exist "%PICO_TOOLCHAIN_PATH%/arm-none-eabi" goto :already_got
set TOOLCHAIN_VERSION=10.3-2021.07
set TOOLCHAIN_BASE_URL=https://developer.arm.com/-/media/Files/downloads/gnu-rm
set TOOLCHAIN_NAME=gcc-arm-none-eabi-%TOOLCHAIN_VERSION%
set TOOLCHAIN_FILE=%TOOLCHAIN_NAME%-win32.zip
curl -Lo tmp.zip %TOOLCHAIN_BASE_URL%/%TOOLCHAIN_VERSION%/%TOOLCHAIN_FILE% || goto :EOF
7z -o"%PICO_TOOLCHAIN_PATH%-tmp" x tmp.zip || goto :EOF
del tmp.zip
move "%PICO_TOOLCHAIN_PATH%-tmp/%TOOLCHAIN_NAME%" "%PICO_TOOLCHAIN_PATH%"
rmdir "%PICO_TOOLCHAIN_PATH%-tmp"
goto :EOF


:already_got
echo.
echo ** Skipping Rp2040 tools installation: '%PICO_TOOLCHAIN_PATH%' exists
echo.
