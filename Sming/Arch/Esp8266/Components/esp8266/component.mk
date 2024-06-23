# base directory of the ESP8266 SDK package, absolute
COMPONENT_SUBMODULES	:= sdk
SDK_BASE				:= $(COMPONENT_PATH)/sdk

DEBUG_VARS				+= FLASH_INIT_DATA FLASH_INIT_DATA_VCC
FLASH_INIT_DATA			= $(SDK_BASE)/bin/esp_init_data_default.bin
FLASH_INIT_DATA_VCC		= $(SDK_BASE)/bin/esp_init_data_vdd_default.bin

CUSTOM_TARGETS			+= $(FLASH_INIT_DATA) $(FLASH_INIT_DATA_VCC)

# => LWIP basic support required by SDK
ifeq ($(DISABLE_WIFI),1)
COMPONENT_DEPENDS	+= esp-lwip
endif


COMPONENT_VARS += ENABLE_CUSTOM_PHY
ENABLE_CUSTOM_PHY ?= 0
ifeq ($(ENABLE_CUSTOM_PHY),1)
	CFLAGS += -DENABLE_CUSTOM_PHY=1
	LDFLAGS += -Wl,-wrap,register_chipv6_phy -u custom_register_chipv6_phy -u get_adc_mode
endif


$(FLASH_INIT_DATA): $(SDK_BASE)/.submodule
	$(Q) cp -f $(@D)/esp_init_data_default_v08.bin $@

PHY_TOOL := $(COMPONENT_PATH)/Tools/patch-phy-bin.py

$(FLASH_INIT_DATA_VCC): $(FLASH_INIT_DATA)
	$(Q) cp $< $@
	$(Q) $(PYTHON) $(PHY_TOOL) $@

SDK_LIBDIR				:= $(SDK_BASE)/lib
COMPONENT_INCDIRS		:= include .

export SDK_LIBDIR

COMPONENT_DOXYGEN_INPUT := \
	include/gpio.h \
	include/pwm.h \
	include/esp_phy.h

# Crash handler hooks this so debugger can be invoked
EXTRA_LDFLAGS := $(call Wrap,system_restart_local)

#
LIBDIRS += $(SDK_LIBDIR)
EXTRA_LIBS := \
	phy \
	crypto \
	hal

LIBDIRS += $(COMPONENT_PATH)/ld $(SDK_LIBDIR)

# SDK-provided crypto library
# Some routines are available in ROM so strip them out
LIBCRYPTO := $(SDK_LIBDIR)/libcrypto.a
LIBCRYPTO_ORIG := $(LIBCRYPTO:.a=.orig.a)
COMPONENT_TARGETS += $(LIBCRYPTO_ORIG)

# Make backup then modify original
$(COMPONENT_RULE)$(LIBCRYPTO_ORIG): $(LIBCRYPTO)
	cp $^ $@
	$(AR) -d $^ aes-internal-dec.o

# Define linker symbols
EXTRA_LDFLAGS += -Wl,--just-symbols=$(COMPONENT_PATH)/ld/crypto.sym
