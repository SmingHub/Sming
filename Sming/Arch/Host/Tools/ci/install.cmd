REM Host install.cmd

if "%BUILD_DOCS%" == "1" (
python -m pip install -r %SMING_HOME%/../docs/requirements.txt
)
