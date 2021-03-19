## User configurable settings

## [Application id and version] ## 
# Application id
APP_ID ?= "test"

# Application version: string containing only the major and minor version separated by comma
# APP_VERSION := "1.2"
# Application patch version: integer containing only the patch version
# APP_VERSION_PATCH := 3

## [TLS/SSL settings ] ##
# Uncomment the line below to start using SSL
CONFIG_VARS := ENABLE_SSL 
# ENABLE_SSL := Bearssl

# Set this to one if the remote firmware server requires client certificate
# This option is in effect only when ENABLE_SSL is set
CONFIG_VARS += ENABLE_CLIENT_CERTIFICATE
ENABLE_CLIENT_CERTIFICATE ?= 0

## [ Firmware Update Server ] ## 
CONFIG_VARS += MQTT_URL
ifeq ($(MQTT_URL),)
    MQTT_URL := "mqtt://test.mosquitto.org:1883"
    ifneq ($(ENABLE_SSL),)
    	ifneq ($(ENABLE_CLIENT_CERTIFICATE),0)
    		MQTT_URL := "mqtts://test.mosquitto.org:8884"
    	else
    		MQTT_URL := "mqtts://test.mosquitto.org:8883"
    	endif
    endif
endif

CONFIG_VARS += ENABLE_OTA_ADVANCED
ENABLE_OTA_ADVANCED ?= 0

## End of user configurable settings. Don't change anything below this line

COMPONENT_DEPENDS := OtaUpgradeMqtt

## use rboot build mode
RBOOT_ENABLED := 1

## Use standard hardware config with two ROM slots and two SPIFFS partitions
HWCONFIG := spiffs-two-roms

APP_CFLAGS = -DMQTT_URL="\"$(MQTT_URL)"\"   \
			 -DAPP_ID="\"$(APP_ID)"\"       \
			 -DENABLE_CLIENT_CERTIFICATE=$(ENABLE_CLIENT_CERTIFICATE)  \
			 -DENABLE_OTA_ADVANCED=$(ENABLE_OTA_ADVANCED)
			 
ifneq ($(APP_VERSION),)
	APP_CFLAGS += -DAPP_VERSION="\"$(APP_VERSION)"\"         \
				  -DAPP_VERSION_PATCH=$(APP_VERSION_PATCH)
endif