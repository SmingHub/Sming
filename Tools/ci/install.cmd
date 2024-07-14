REM
REM Windows CI install script
REM
REM May also be used for library CI builds. The following variables must be set:
REM
REM   CI_BUILD_DIR
REM   SMING_HOME
REM

if "%SMING_TOOLS_PREINSTALLED%" NEQ "" goto :EOF

if "%BUILD_DOCS%" == "true" (
    set INSTALL_OPTS=doc
)

REM Python and CMake are preconfigured
choco install ninja ccache -y --no-progress || goto :error

call %~dp0..\mingw-install.cmd || goto :error

call %~dp0..\install.cmd %SMING_ARCH% %INSTALL_OPTS% || goto :error

REM Clean up tools installation
python "%~dp0clean-tools.py" clean --delete

goto :EOF


:error
echo Failed with error #%errorlevel%.
exit /b %errorlevel%
