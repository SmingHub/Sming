REM
REM Windows CI install script
REM

if "%SMING_TOOLS_PREINSTALLED%" NEQ "" goto :EOF

if "%BUILD_DOCS%" == "true" (
    set INSTALL_OPTS=doc
)

call %CI_BUILD_DIR%\Tools\install.cmd %SMING_ARCH% %INSTALL_OPTS%

python "%CI_BUILD_DIR%\Tools\ci\clean-tools.py" clean --delete
