
# => SMING

MODULES			+= $(ARCH_COMPONENTS)/esp8266
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp8266/include $(SDK_INCDIR)


## => Makefile-bsd.mk
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

## => Makefile-linux.mk
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

## => Makefile-macos.mk
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

## => Makefile-windows.mk
# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS	 ?= $(ESP_HOME)/utils


# => SMING

ifeq ($(SDK_INTERNAL), 1)
	SUBMODULES		+= $(SDK_COMPONENT)
	CUSTOM_TARGETS	+= $(SDK_COMPONENT)/bin/esp_init_data_default.bin

$(SDK_COMPONENT)/bin/esp_init_data_default.bin:
	$(Q) cp -f $(dir $@)esp_init_data_default_v08.bin $@
endif


# => 'Internal' SDK - for SDK Version 3+ as submodule in Sming repository
# SDK_BASE just needs to point into our repo as it's overridden with the correct submodule path
# This provides backward-compatiblity, so $(SMING)/third-party/ESP8266_NONOS_SDK) still works
CONFIG_VARS += SDK_BASE SDK_INTERNAL
SDK_BASE := $(call FixPath,$(abspath $(SDK_BASE)))
ifneq (,$(findstring $(SMING_HOME),$(SDK_BASE)))
	SDK_COMPONENT	:= $(ARCH_COMPONENTS)/Sdk/ESP8266_NONOS_SDK
	SDK_BASE		:= $(SMING_HOME)/$(SDK_COMPONENT)
	CFLAGS			+= -DSDK_INTERNAL
	SDK_INTERNAL = 1
else
	SDK_INTERNAL = 0
endif

# various paths from the SDK used in this project
SDK_LIBDIR	:= $(SDK_BASE)/lib
SDK_INCDIR	:= $(SDK_BASE)/include



# => APP

EXTRA_INCDIR += $(ARCH_COMPONENTS)/esp8266/include $(SDK_INCDIR)



