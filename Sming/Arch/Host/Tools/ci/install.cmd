REM Host install.cmd

call :install "c:\tools\doxygen" doxygen-1.9.1.windows.bin.zip
call :install "c:\tools" stable_windows_10_msbuild_Release_Win32_graphviz-2.46.1-win32.zip

python -m pip install -r %SMING_HOME%/../docs/requirements.txt

goto :EOF

:install
if "%~1"=="" goto :EOF
mkdir %1
curl -LO %SMINGTOOLS%/%2
7z -o%1 -y x %2

where libxcb.dll
