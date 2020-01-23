# base directory of the ESP8266 SDK package, absolute
COMPONENT_VARS			:= SDK_BASE

SDK_BASE ?= $(COMPONENT_PATH)/ESP8266_NONOS_SDK

SDK_BASE				:= $(call FixPath,$(SDK_BASE))
FLASH_INIT_DATA			= $(SDK_BASE)/bin/esp_init_data_default.bin

# => 'Internal' SDK - for SDK Version 3+ as submodule in Sming repository
# SDK_BASE just needs to point into our repo as it's overridden with the correct submodule path
# This provides backward-compatiblity, so $(SMING)/third-party/ESP8266_NONOS_SDK) still works
DEBUG_VARS				+= SDK_INTERNAL
ifneq (,$(findstring $(SMING_HOME),$(SDK_BASE)))
GLOBAL_CFLAGS			+= -DSDK_INTERNAL=1
SDK_INTERNAL			:= 1
COMPONENT_SUBMODULES	:= ESP8266_NONOS_SDK
SDK_BASE				:= $(COMPONENT_PATH)/ESP8266_NONOS_SDK

$(FLASH_INIT_DATA): $(SDK_BASE)/.submodule
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

DOXYGEN_INPUT += \
	$(SDK_INCDIR)/gpio.h \
	$(SDK_INCDIR)/pwm.h

# Crash handler hooks this so debugger can be invoked
EXTRA_LDFLAGS := -Wl,-wrap,system_restart_local

#
LIBDIRS += $(SDK_LIBDIR)
EXTRA_LIBS := \
	phy \
	crypto \
	hal

LIBDIRS += $(COMPONENT_PATH)/ld $(SDK_BASE)/ld $(SDK_LIBDIR)

# SDK-provided crypto library
# Some routines are available in ROM so strip them out
LIBCRYPTO := $(SDK_LIBDIR)/libcrypto.a
LIBCRYPTO_ORIG := $(LIBCRYPTO:.a=.orig.a)
COMPONENT_TARGETS += $(LIBCRYPTO_ORIG)

# Make backup then modify original
$(COMPONENT_RULE)$(LIBCRYPTO_ORIG): $(LIBCRYPTO)
	cp $^ $@
	ar -d $^ aes-internal-dec.o

# Define linker symbols
EXTRA_LDFLAGS += -Wl,--just-symbols=$(COMPONENT_PATH)/ld/crypto.sym
