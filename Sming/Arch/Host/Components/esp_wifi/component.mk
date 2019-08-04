COMPONENT_DEPENDS	:= lwip

# Options to add for configuring host network behaviour
CACHE_VARS				+= HOST_NETWORK_OPTIONS
HOST_NETWORK_OPTIONS	?=
CLI_TARGET_OPTIONS		+= $(HOST_NETWORK_OPTIONS)

COMPONENT_TARGETS		:= esp-wifi-check

.PHONY: esp-wifi-check
$(COMPONENT_RULE)esp-wifi-check:
ifeq ($(ENABLE_WPS),1)
	$(warning WPS not supported)
endif
ifeq ($(ENABLE_SMART_CONFIG),1)
	$(warning 'Smart Config' not supported)
endif
