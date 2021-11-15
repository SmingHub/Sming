COMPONENT_SOC := esp* host
HWCONFIG := standard-4m

ifdef MQTT_URL
	USER_CFLAGS += -DMQTT_URL=\"$(MQTT_URL)\" 
endif
