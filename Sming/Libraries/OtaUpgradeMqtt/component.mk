COMPONENT_SRCDIRS := src/
COMPONENT_INCDIRS := src/include

# If enabled (set to 1) then we can use all sofisticated mechanisms to upgrade the firmware using the ``OtaUpgrade`` library.
COMPONENT_VARS := ENABLE_OTA_ADVANCED
ENABLE_OTA_ADVANCED ?= 0

# If enabled (set to 1) then we can use unlimited number of patch versions
COMPONENT_VARS += ENABLE_OTA_VARINT_VERSION 
ENABLE_OTA_VARINT_VERSION ?= 1

ifneq ($(ENABLE_OTA_ADVANCED),0)
	COMPONENT_DEPENDS := OtaUpgrade	
endif