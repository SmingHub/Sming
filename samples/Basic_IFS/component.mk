COMPONENT_SOC := esp* host
COMPONENT_DEPENDS := LittleFS

# Empty SPIFFS partition please
SPIFF_FILES :=

# Use to store filesystem image in a FlashString object instead of partition
CONFIG_VARS += ENABLE_FLASHSTRING_IMAGE
ENABLE_FLASHSTRING_IMAGE ?= 0
ifeq ($(ENABLE_FLASHSTRING_IMAGE),1)
COMPONENT_CXXFLAGS += -DENABLE_FLASHSTRING_IMAGE=1
HWCONFIG := spiffs
else
HWCONFIG := basic_ifs_$(SMING_ARCH)
endif
