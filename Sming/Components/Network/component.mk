COMPONENT_SRCDIRS := \
	src \
	$(call ListAllSubDirs,$(COMPONENT_PATH)/src) \
	$(call ListAllSubDirs,$(COMPONENT_PATH)/Arch/$(SMING_ARCH))

COMPONENT_INCDIRS := src

COMPONENT_DOXYGEN_INPUT := src

COMPONENT_DEPENDS := \
	ssl \
	http-parser \
	libb64 \
	ws_parser \
	mqtt-codec \
	libyuarel
