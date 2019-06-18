# => SMING

ifeq ($(SMING_RELEASE),1)
	CFLAGS += -DLWIP_NOASSERT
endif

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





# => APP

CONFIG_VARS			+= ENABLE_CUSTOM_LWIP LIBLWIP
ENABLE_CUSTOM_LWIP	?= 1
ifeq ($(ENABLE_CUSTOM_LWIP), 0)
	LIBLWIP			:= lwip
	LWIP_BASE		:= $(ARCH_COMPONENTS)/esp-lwip
	EXTRA_INCDIR	+= $(LWIP_BASE)/include $(LWIP_BASE)
else ifeq ($(ENABLE_CUSTOM_LWIP), 1)
	LIBLWIP			:= lwip_open
	LWIP_BASE		:= $(ARCH_COMPONENTS)/esp-open-lwip/esp-open-lwip
	EXTRA_INCDIR	+= $(LWIP_BASE)/include
	ifeq ($(ENABLE_ESPCONN), 1)
		LIBLWIP		:= lwip_full
	endif
else ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	ifeq ($(ENABLE_ESPCONN), 1)
		$(error LWIP2 does not support espconn_* functions. Make sure to set ENABLE_CUSTOM_LWIP to 0 or 1.)
	endif
	LIBLWIP			:= lwip2
	LWIP_BASE		:= $(ARCH_COMPONENTS)/lwip2/lwip2
	EXTRA_INCDIR	+= $(LWIP_BASE)/glue-esp/include-esp $(LWIP_BASE)/include
else
	EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp-lwip/lwip/include
endif
LIBS				+= $(LIBLWIP)

ifneq ($(ENABLE_CUSTOM_LWIP), 0)
	LIBLWIP_DST		:= $(call UserLibPath,$(LIBLWIP))
	CUSTOM_TARGETS	+= $(LIBLWIP_DST)

$(LIBLWIP_DST):
	$(call MakeLibrary,$@,ENABLE_CUSTOM_LWIP=$(ENABLE_CUSTOM_LWIP) ENABLE_ESPCONN=$(ENABLE_ESPCONN))
endif

