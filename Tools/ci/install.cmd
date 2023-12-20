REM
REM Windows CI install script
REM

if "%SMING_TOOLS_PREINSTALLED%" NEQ "" goto :EOF

if "%BUILD_DOCS%" == "true" (
    set INSTALL_OPTS=doc
)

choco install ninja

%SMING_HOME%\..\Tools\install.cmd %SMING_ARCH% %INSTALL_OPTS%
