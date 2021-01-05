# Esp8266 build.run.sh

run_esp8266() {
	export ESP_HOME=$1
	make -C "$SMING_PROJECTS_DIR/samples/HttpServer_FirmwareUpload" python-requirements
	$MAKE_PARALLEL samples
	make clean samples-clean
	$MAKE_PARALLEL Basic_Blink ENABLE_CUSTOM_HEAP=1 DEBUG_VERBOSE_LEVEL=3
	$MAKE_PARALLEL HttpServer_ConfigNetwork ENABLE_CUSTOM_LWIP=2 STRICT=1
}

run_esp8266 $UDK_ROOT
run_esp8266 $EQT_ROOT
