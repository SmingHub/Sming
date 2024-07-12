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

call %SMING_HOME%\..\Tools\install.cmd %SMING_ARCH% %INSTALL_OPTS%

REM Clean up tools installation
python "%SMING_HOME%\..\Tools\ci\clean-tools.py" clean --delete
