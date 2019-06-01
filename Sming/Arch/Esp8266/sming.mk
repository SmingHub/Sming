# ESP8266 Sming framework

# => SDK
ifeq ($(SDK_INTERNAL), 1)
	SUBMODULES		+= $(SDK_COMPONENT)
	CUSTOM_TARGETS	+= $(SDK_COMPONENT)/bin/esp_init_data_default.bin

$(SDK_COMPONENT)/bin/esp_init_data_default.bin:
	$(Q) cp -f $(dir $@)esp_init_data_default_v08.bin $@
endif

MODULES			+= $(ARCH_COMPONENTS)/esp8266
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp8266/include $(SDK_INCDIR)

MODULES			+= $(ARCH_COMPONENTS)/spi_flash
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/spi_flash/include

MODULES			+= $(ARCH_COMPONENTS)/driver
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/driver/include

MODULES			+= $(ARCH_COMPONENTS)/esp_wifi
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp_wifi/include

MODULES			+= $(ARCH_COMPONENTS)/fatfs

# => rboot
RBOOT_BASE		:= $(ARCH_COMPONENTS)/rboot/rboot
SUBMODULES		+= $(RBOOT_BASE)
EXTRA_INCDIR	+= $(RBOOT_BASE) $(RBOOT_BASE)/appcode

# => SPIFFS
SPIFFS_SMING	:= $(ARCH_COMPONENTS)/spiffs
SPIFFS_BASE		:= $(COMPONENTS)/spiffs
SUBMODULES		+= $(SPIFFS_BASE)
MODULES			+= $(SPIFFS_SMING) $(SPIFFS_BASE)/src
EXTRA_INCDIR	+= $(SPIFFS_SMING) $(SPIFFS_BASE)/src


# => ESP8266_new_pwm
CONFIG_VARS += ENABLE_CUSTOM_PWM
ENABLE_CUSTOM_PWM	?= 1
ifeq ($(ENABLE_CUSTOM_PWM), 1)
	PWM_BASE		:= $(ARCH_COMPONENTS)/pwm/new-pwm
	SUBMODULES		+= $(PWM_BASE)
	CFLAGS			+= -DSDK_PWM_PERIOD_COMPAT_MODE=1
	LIBPWM			:= pwm_open
	LIBS			+= $(LIBPWM)
	CLEAN			+= pwm-clean

$(call UserLibPath,$(LIBPWM)): | $(PWM_BASE)/.submodule
	$(Q) $(CC) $(INCDIR) $(CFLAGS) -c $(PWM_BASE)/pwm.c -o $(PWM_BASE)/pwm.o
	$(Q) $(AR) rcs $@ $(PWM_BASE)/pwm.o

.PHONY: pwm-clean
pwm-clean:
	-$(Q) rm -f $(PWM_BASE)/*.o
endif


# => umm_malloc (custom heap allocation)
CONFIG_VARS += ENABLE_CUSTOM_HEAP
ENABLE_CUSTOM_HEAP		?= 0
ifeq ($(ENABLE_CUSTOM_HEAP), 1)
	CUSTOM_HEAP_BASE	:= $(ARCH_COMPONENTS)/custom_heap
	UMM_MALLOC_BASE		:= $(CUSTOM_HEAP_BASE)/umm_malloc
	SUBMODULES			+= $(UMM_MALLOC_BASE)
	MODULES				+= $(CUSTOM_HEAP_BASE) $(UMM_MALLOC_BASE)/src
	EXTRA_INCDIR		+= $(UMM_MALLOC_BASE)/src $(UMM_MALLOC_BASE)/includes/c-helper-macros
	LIBMAINMM			:= mainmm
	LIBS				+= $(LIBMAINMM)

# Make copy of libmain and remove mem_manager.o module
$(call UserLibPath,$(LIBMAINMM)): $(SDK_LIBDIR)/libmain.a | $(UMM_MALLOC_BASE)/.submodule
	$(vecho) "Enabling custom heap implementation"
	$(Q) cp $^ $@
	$(Q) $(AR) -d $@ mem_manager.o
endif


# => Open Source LWIP
CONFIG_VARS += ENABLE_CUSTOM_LWIP ENABLE_LWIP_DEBUG ENABLE_ESPCONN
ENABLE_CUSTOM_LWIP	?= 1
ENABLE_ESPCONN		?= 0
ifeq ($(ENABLE_CUSTOM_LWIP), 0)
	LIBLWIP			:= lwip
	LWIP_BASE		:= $(ARCH_COMPONENTS)/esp-lwip
	EXTRA_INCDIR	+= $(LWIP_BASE)/include $(LWIP_BASE)
else
	EXTRA_CFLAGS_LWIP 	:= -I$(SMING_HOME)/System/include -I$(ARCH_SYS)/include -I$(ARCH_COMPONENTS)/esp8266/include -I$(SMING_HOME)/Wiring
	ENABLE_LWIPDEBUG ?= 0
	ifeq ($(ENABLE_LWIPDEBUG), 1)
		EXTRA_CFLAGS_LWIP += -DLWIP_DEBUG
	endif

	ifeq ($(ENABLE_CUSTOM_LWIP), 1)
		LWIP_BASE		:= $(ARCH_COMPONENTS)/esp-open-lwip/esp-open-lwip
		SUBMODULES		+= $(LWIP_BASE)
		EXTRA_INCDIR	+= $(LWIP_BASE)/include
		ifeq ($(ENABLE_ESPCONN), 1)
			LIBLWIP		:= lwip_full
		else
			LIBLWIP		:= lwip_open
		endif
		LWIP_BUILD = $(MAKE) -C $(LWIP_BASE) -f Makefile.open ENABLE_ESPCONN=$(ENABLE_ESPCONN) SDK_BASE=$(SDK_BASE) \
			USER_LIBDIR="$(SMING_HOME)/$(USER_LIBDIR)/" CFLAGS_EXTRA="$(EXTRA_CFLAGS_LWIP) $(CFLAGS_COMMON)"
	else ifeq ($(ENABLE_CUSTOM_LWIP), 2)
		ifeq ($(ENABLE_ESPCONN), 1)
			$(error LWIP2 does not support espconn_* functions. Make sure to set ENABLE_CUSTOM_LWIP to 0 or 1.)
		endif
		LWIP_BASE		:= $(ARCH_COMPONENTS)/lwip2/lwip2
		SUBMODULES		+= $(LWIP_BASE)
		EXTRA_INCDIR	+= $(LWIP_BASE)/glue-esp/include-esp $(LWIP_BASE)/include
		LIBLWIP			?= lwip2
		LWIP_BUILD		:= $(MAKE) -C $(LWIP_BASE) -f Makefile.sming ENABLE_ESPCONN=$(ENABLE_ESPCONN) \
						SDK_BASE=$(SDK_BASE) SDK_INTERNAL=$(SDK_INTERNAL) \
						USER_LIBDIR="$(SMING_HOME)/$(USER_LIBDIR)/" CFLAGS_EXTRA="$(EXTRA_CFLAGS_LWIP)"
	endif
	LIBS				+= $(LIBLWIP)
	CLEAN 				+= lwip-clean

$(call UserLibPath,lwip%): | $(LWIP_BASE)/.submodule
	$(vecho) "Building $(notdir $@)..."
	$(Q) $(LWIP_BUILD) CC=$(CC) AR=$(AR) all

.PHONY: lwip-clean
lwip-clean:
	-$(Q) -$(LWIP_BUILD) clean

endif

# => SSL support using axTLS
CONFIG_VARS += ENABLE_SSL SSL_DEBUG
ENABLE_SSL ?= 0
ifeq ($(ENABLE_SSL),1)
	AXTLS_BASE		:= $(ARCH_COMPONENTS)/axtls-8266/axtls-8266
	SUBMODULES		+= $(AXTLS_BASE)
	LIBAXTLS		:= axtls
	LIBS			+= $(LIBAXTLS)
	MODULES			+= $(AXTLS_BASE)/compat $(AXTLS_BASE)/replacements
	EXTRA_INCDIR	+= $(AXTLS_BASE)/.. $(AXTLS_BASE) $(AXTLS_BASE)/ssl $(AXTLS_BASE)/crypto
	AXTLS_FLAGS		= -DLWIP_RAW=1 -DENABLE_SSL=1
	ifeq ($(SSL_DEBUG),1) #
		AXTLS_FLAGS	+= -DSSL_DEBUG=1 -DDEBUG_TLS_MEM=1 -DAXL_DEBUG=1
	endif
	CLEAN			+= axtls-clean
	CFLAGS			+= $(AXTLS_FLAGS)
	AXTLS_BUILD		:= $(MAKE) -C $(AXTLS_BASE) -e V=$(V) BIN_DIR="$(SMING_HOME)/$(USER_LIBDIR)"

$(call UserLibPath,$(LIBAXTLS)): | $(AXTLS_BASE)/.submodule
	$(Q) $(AXTLS_BUILD) all

.PHONY: axtls-clean
axtls-clean:
	-$(Q) $(AXTLS_BUILD) clean
endif


# Tools

#
# $1 -> target
# $2 -> make directory
# $3 -> parameters
#
define make-tool
	$(Q) mkdir -p $(BUILD_BASE)/$(2)/$(UNAME)
	$(MAKE) --no-print-directory -C $2 TARGET=$(abspath $1) BUILD_DIR=$(abspath $(BUILD_BASE)/$(2)/$(UNAME)) V=$(V) $3
endef

# => spiffy
TOOLS			+= $(SPIFFY)
TOOLS_CLEAN		+= spiffy-clean

SPIFFY_BASE := $(ARCH_TOOLS)/spiffy
$(SPIFFY): | $(COMPONENTS)/spiffs/.submodule
	$(Q) $(call make-tool,$@,$(SPIFFY_BASE),SPIFFS_SMING=$(SMING_HOME)/$(SPIFFS_SMING) SPIFFS_BASE=$(SMING_HOME)/$(SPIFFS_BASE))

.PHONY: spiffy-clean
spiffy-clean:
	-$(Q) -$(call make-tool,$(SPIFFY),$(SPIFFY_BASE),clean)

# => esptool2
TOOLS			+= $(ESPTOOL2)
TOOLS_CLEAN		+= esptool2-clean

ESPTOOL2_BASE := $(ARCH_TOOLS)/esptool2
SUBMODULES += $(ESPTOOL2_BASE)
$(ESPTOOL2): | $(ESPTOOL2_BASE)/.submodule
	$(Q) $(call make-tool,$@,$(ESPTOOL2_BASE))

.PHONY: esptool2-clean
esptool2-clean:
	-$(Q) -$(call make-tool,$(ESPTOOL2),$(ESPTOOL2_BASE),clean)

