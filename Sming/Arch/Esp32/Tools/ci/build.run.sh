# Esp32 build.run.sh

$MAKE_PARALLEL Basic_Blink Basic_WiFi HttpServer_ConfigNetwork DEBUG_VERBOSE_LEVEL=3 STRICT=1
$MAKE_PARALLEL Basic_Ssl ENABLE_SSL=Bearssl DEBUG_VERBOSE_LEVEL=3 STRICT=1

# esp32s2
make clean components-clean
$MAKE_PARALLEL Basic_Blink ESP_VARIANT=esp32s2

# esp32c3
make clean components-clean
$MAKE_PARALLEL Basic_Blink ESP_VARIANT=esp32c3

# esp32s3
make clean components-clean
$MAKE_PARALLEL Basic_Blink ESP_VARIANT=esp32s3
