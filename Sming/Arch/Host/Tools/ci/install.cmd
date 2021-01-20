REM Host install.cmd

curl -LO https://doxygen.nl/files/doxygen-1.9.1-setup.exe
doxygen-1.9.1-setup /silent

choco install -y graphviz

python -m pip install -r %SMING_HOME%/../docs/requirements.txt
