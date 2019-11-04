COMPONENT_LIBNAME	:=

COMPONENT_DEPENDS	:= lwip

# Options to add for configuring host network behaviour
CACHE_VARS				+= HOST_NETWORK_OPTIONS
HOST_NETWORK_OPTIONS	?=
CLI_TARGET_OPTIONS		+= $(HOST_NETWORK_OPTIONS)

App-build: esp-wifi-check

.PHONY: esp-wifi-check
esp-wifi-check:
ifeq ($(ENABLE_WPS),1)
	$(error WPS not supported)
endif
ifeq ($(ENABLE_SMART_CONFIG),1)
	$(error 'Smart Config' not supported)
endif
