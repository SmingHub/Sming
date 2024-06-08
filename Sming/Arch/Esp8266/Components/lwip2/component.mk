# For now, build as a regular library using existing makefile (variants not supported)
COMPONENT_LIBNAME :=

COMPONENT_DEPENDS		:= esp8266

ifeq ($(SMING_RELEASE),1)
	COMPONENT_CFLAGS := \
		-DLWIP_NOASSERT \
		-Wno-implicit-function-declaration
endif

COMPONENT_VARS		:= ENABLE_LWIPDEBUG ENABLE_ESPCONN
ifneq ($(ENABLE_LWIPDEBUG),1)
override ENABLE_LWIPDEBUG := 0
endif
ENABLE_ESPCONN		?= 0

EXTRA_CFLAGS_LWIP  := \
	-I$(SMING_HOME)/System/include \
	-I$(ARCH_COMPONENTS)/esp8266/include \
	-I$(ARCH_COMPONENTS)/libc/include \
	-DULWIPDEBUG=$(ENABLE_LWIPDEBUG)


COMPONENT_SUBMODULES := lwip2
COMPONENT_INCDIRS := \
	lwip2/glue-lwip/arduino \
	lwip2/glue-lwip \
	lwip2/glue \
	lwip2/lwip2-src/src/include

LWIP2_PATH := $(COMPONENT_PATH)/lwip2
LWIP2_LIBPATH := $(COMPONENT_LIBDIR)

#
COMPONENT_VARS += TCP_MSS LWIP_IPV6 LWIP_FEATURES
TCP_MSS ?= 1460
ifneq ($(LWIP_IPV6),1)
override LWIP_IPV6 := 0
endif
ifneq ($(LWIP_FEATURES),1)
LWIP_FEATURES := 0
endif

LWIP2_HASHVAL   := $(foreach v,$(COMPONENT_VARS),$v=$($v))
LWIP2_LIBHASH   := $(call CalculateVariantHash,LWIP2_HASHVAL)
LWIP2_BUILD_DIR := $(COMPONENT_BUILD_BASE)/$(LWIP2_LIBHASH)

GLOBAL_CFLAGS += \
	-DTCP_MSS=$(TCP_MSS) \
	-DLWIP_IPV6=$(LWIP_IPV6) \
	-DLWIP_FEATURES=$(LWIP_FEATURES) \
	-DULWIPDEBUG=$(ENABLE_LWIPDEBUG)

# Make is pretty complex for LWIP2, and mucks about with output sections so build as a regular library
LWIP2_LIB			:= $(COMPONENT_NAME)
LWIP2_TARGET		:= $(LWIP2_BUILD_DIR)/lib$(LWIP2_LIB).a
COMPONENT_TARGETS	:= $(LWIP2_TARGET)
EXTRA_LIBS			:= $(LWIP2_LIB)

LIBDIRS += $(LWIP2_BUILD_DIR)

COMPONENT_PREREQUISITES := $(LWIP2_PATH)/glue-lwip/lwip-err-t.h

$(LWIP2_PATH)/glue-lwip/lwip-err-t.h:
	$(Q) $(MAKE) -C $(LWIP2_PATH) -f ../Makefile.sming patch

$(COMPONENT_RULE)$(LWIP2_TARGET):
	$(Q) $(MAKE) --no-print-directory -C $(LWIP2_PATH) -f ../Makefile.sming \
		all \
		CFLAGS_EXTRA="$(EXTRA_CFLAGS_LWIP)" \
		LWIP_LIB_RELEASE=$(LWIP2_TARGET) \
		TOOLS=$(TOOLSPEC) \
		TCP_MSS=$(TCP_MSS) \
		LWIP_FEATURES=$(LWIP_FEATURES) \
		LWIP_IPV6=$(LWIP_IPV6) \
		BUILD=$(LWIP2_BUILD_DIR)/build \
		Q=$(Q) V=$(if $V,1,0)
