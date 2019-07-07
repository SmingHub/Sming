DISABLE_SPIFFS = 1
DEBUG_VERBOSE_LEVEL = 3
SPI_SIZE = 4M
ARDUINO_LIBRARIES := ArduinoJson5 ArduinoJson6

.PHONY: execute
execute: flash run
