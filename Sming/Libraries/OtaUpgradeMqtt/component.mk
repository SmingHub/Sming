COMPONENT_SRCDIRS  := 
COMPONENT_SRCFILES := src/PayloadParser.cpp src/RbootPayloadParser.cpp 
COMPONENT_INCDIRS  := src/include

# If enabled (set to 1) then we can use all sofisticated mechanisms to upgrade the firmware using the ``OtaUpgrade`` library.
COMPONENT_VARS := ENABLE_OTA_ADVANCED
ENABLE_OTA_ADVANCED ?= 0

ifneq ($(ENABLE_OTA_ADVANCED),0)
	COMPONENT_SRCFILES += src/AdvancedPayloadParser.cpp
	COMPONENT_DEPENDS  := OtaUpgrade
endif

# If enabled (set to 1) then we can use unlimited number of patch versions
COMPONENT_VARS += ENABLE_OTA_VARINT_VERSION 
ENABLE_OTA_VARINT_VERSION ?= 1

COMPONENT_CXXFLAGS := -DENABLE_OTA_ADVANCED=$(ENABLE_OTA_ADVANCED) \
					  -DENABLE_OTA_VARINT_VERSION=$(ENABLE_OTA_VARINT_VERSION)