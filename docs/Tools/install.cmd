REM Docs install.cmd

echo.
echo ** Installing documentation tools
echo.

where /q doxygen
if errorlevel 1 (
    choco install -y --no-progress doxygen.install graphviz gtk-runtime
)

python -m pip install --upgrade pip wheel
python -m pip install -r %~dp0..\requirements.txt
python -m pip uninstall -y xcffib
