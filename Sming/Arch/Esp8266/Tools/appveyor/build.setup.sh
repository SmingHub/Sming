unset SPIFFY
unset ESPTOOL2
unset SDK_BASE

if [ "$build_compiler" == "udk" ]; then
  export ESP_HOME=$APPVEYOR_BUILD_FOLDER/opt/esp-alt-sdk
elif [ "$build_compiler" == "eqt"]; then
  export ESP_HOME=$APPVEYOR_BUILD_FOLDER/opt/esp-quick-toolchain
endif
