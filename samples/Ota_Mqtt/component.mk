## User configurable settings

ENABLE_CLIENT_CERTIFICATE ?= 0

CONFIG_VARS := ENABLE_SSL ENABLE_CLIENT_CERTIFICATE 

# Application id
APP_ID := "test"

# Application version: string containing only the major and minor version separated by comma
# APP_VERSION := "1.2"
# Application patch version: integer containing only the patch version
# APP_VERSION_PATCH := 3

# Firmware Update Server 
MQTT_URL := "mqtt://test.mosquitto.org:1883"
ifneq ($(ENABLE_SSL),)
	ifneq ($(ENABLE_CLIENT_CERTIFICATE),0)
		MQTT_URL := "mqtts://test.mosquitto.org:8884"
	else
		MQTT_URL := "mqtts://test.mosquitto.org:8883"
	endif
endif

## End of user configurable settings. Don't change anything below this line

## use rboot build mode
RBOOT_ENABLED := 1

## Use standard hardware config with two ROM slots and two SPIFFS partitions
ifeq ($(SMING_ARCH),Host)
	HWCONFIG := basic_rboot_host
else
	HWCONFIG := basic_rboot
endif

APP_CFLAGS = -DMQTT_URL="\"$(MQTT_URL)"\" -DAPP_ID="\"$(APP_ID)"\" -DENABLE_CLIENT_CERTIFICATE=$(ENABLE_CLIENT_CERTIFICATE)
ifneq ($(APP_VERSION),)
	APP_CFLAGS += -DAPP_VERSION="\"$(APP_VERSION)"\"
endif 