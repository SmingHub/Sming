REM
REM Windows CI install script
REM

if "%SMING_TOOLS_PREINSTALLED%" NEQ "" goto :EOF

if "%BUILD_DOCS%" == "true" (
    set INSTALL_OPTS=doc
)

call %CI_BUILD_DIR%\Tools\install.cmd %SMING_ARCH% %INSTALL_OPTS%

REM Configure ccache
ccache --set-config cache_dir="%CI_BUILD_DIR%\.ccache"
ccache --set-config max_size=500M
ccache -z

REM Clean up tools installation
python "%CI_BUILD_DIR%\Tools\ci\clean-tools.py" clean --delete
