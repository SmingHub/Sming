# Esp32 build.run.sh

$MAKE_PARALLEL Basic_Blink Basic_WiFi HttpServer_ConfigNetwork DEBUG_VERBOSE_LEVEL=3 STRICT=1
$MAKE_PARALLEL Basic_Ssl ENABLE_SSL=Bearssl DEBUG_VERBOSE_LEVEL=3 STRICT=1

ESP32_PROJECTS="Basic_Blink Basic_Ethernet"

# esp32s2
$MAKE_PARALLEL ESP_VARIANT=esp32s2 $ESP32_PROJECTS

# esp32c3
$MAKE_PARALLEL ESP_VARIANT=esp32c3 $ESP32_PROJECTS

# esp32s3
$MAKE_PARALLEL ESP_VARIANT=esp32s3 $ESP32_PROJECTS

# make sure that the Ota Library sample compiles for ESP32
$MAKE_PARALLEL -C "$SMING_HOME/Libraries/OtaUpgradeMqtt/samples/Upgrade"