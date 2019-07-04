# base directory of the ESP8266 SDK package, absolute
COMPONENT_VARS			:= SDK_BASE

ifeq ($(UNAME),Windows)
SDK_BASE				?= $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS				?= $(ESP_HOME)/utils
else
SDK_BASE				?= $(ESP_HOME)/sdk
SDK_TOOLS				?= $(SDK_BASE)/tools
endif

SDK_BASE				:= $(call FixPath,$(SDK_BASE))

# => 'Internal' SDK - for SDK Version 3+ as submodule in Sming repository
# SDK_BASE just needs to point into our repo as it's overridden with the correct submodule path
# This provides backward-compatiblity, so $(SMING)/third-party/ESP8266_NONOS_SDK) still works
DEBUG_VARS				+= SDK_INTERNAL
ifneq (,$(findstring $(SMING_HOME),$(SDK_BASE)))
GLOBAL_CFLAGS			+= -DSDK_INTERNAL=1
SDK_INTERNAL			:= 1
COMPONENT_SUBMODULES	:= ESP8266_NONOS_SDK
SDK_BASE				:= $(COMPONENT_PATH)/ESP8266_NONOS_SDK
COMPONENT_TARGETS		:= $(SDK_BASE)/bin/esp_init_data_default.bin

$(COMPONENT_RULE)$(SDK_BASE)/bin/esp_init_data_default.bin:
	$(Q) cp -f $(@D)/esp_init_data_default_v08.bin $@
else
SDK_INTERNAL			:= 0
endif

DEBUG_VARS				+= SDK_LIBDIR SDK_INCDIR
SDK_LIBDIR				:= $(SDK_BASE)/lib
SDK_INCDIR				:= $(SDK_BASE)/include
COMPONENT_INCDIRS		:= include $(SDK_INCDIR)

export SDK_INTERNAL
export SDK_LIBDIR
export SDK_INCDIR

# Required to ensure user-provided function gets linked instead of our 'weak' one
COMPONENT_LDFLAGS += -u custom_crash_callback

# Crash handler hooks this so debugger can be invoked
COMPONENT_LDFLAGS += -Wl,-wrap,system_restart_local 
