DISABLE_SPIFFS = 1

ifdef MQTT_URL
	USER_CFLAGS += -DMQTT_URL=\"$(MQTT_URL)\" 
endif

## size of the flash chip
SPI_SIZE  ?= 4M
