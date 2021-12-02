REM Docs install.cmd

choco install -y doxygen.install graphviz
setx PATH "%PROGRAMFILES%\doxygen\bin;%PROGRAMFILES%\Graphviz\bin;%PATH%"

python -m pip install --upgrade pip wheel
python -m pip install -r %~dp0/../requirements.txt
python -m pip uninstall -y xcffib
