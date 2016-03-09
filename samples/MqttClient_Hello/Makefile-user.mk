## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

## Add your source directories here separated by space
# MODULES = app
# EXTRA_INCDIR = include

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

## MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## Serial port parameter is required to flash firmware correctly.
## Windows: 
# SERIAL_PORT = COM3

## MacOS / Linux:
# SERIAL_PORT = /dev/tty.usbserial

## Serial port baudrate
# SERIAL_BAUDRATE	= 115200

## Configure flash parameters (for ESP12-E and other new boards):
# SPI_MODE = dio

## SPIFFS options
DISABLE_SPIFFS = 1
# SPIFF_FILES = files
# we will use global WiFi settings from Eclipse Environment Variables, if possible

ifdef MQTT_USERNAME
	USER_CFLAGS += -DMQTT_USERNAME=\"$(MQTT_USERNAME)\" -DMQTT_PWD=\"$(MQTT_PWD)\"
endif

ifdef MQTT_HOST
	USER_CFLAGS += -DMQTT_HOST=\"$(MQTT_HOST)\"
endif

ifdef MQTT_PORT
	USER_CFLAGS += -DMQTT_PORT=$(MQTT_PORT)
endif

