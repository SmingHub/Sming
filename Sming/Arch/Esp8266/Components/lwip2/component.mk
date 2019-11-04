# For now, build as a regular library using existing makefile (variants not supported)
COMPONENT_LIBNAME :=

COMPONENT_DEPENDS		:= esp8266

ifeq ($(SMING_RELEASE),1)
	COMPONENT_CFLAGS := \
		-DLWIP_NOASSERT \
		-Wno-implicit-function-declaration
endif

COMPONENT_VARS		:= ENABLE_LWIPDEBUG ENABLE_ESPCONN
ENABLE_LWIPDEBUG	?= 0
ENABLE_ESPCONN		?= 0

EXTRA_CFLAGS_LWIP  := \
	-I$(SMING_HOME)/System/include \
	-I$(ARCH_SYS)/include \
	-I$(ARCH_COMPONENTS)/esp8266/include \
	-I$(SMING_HOME)/Wiring

ifeq ($(ENABLE_LWIPDEBUG), 1)
	EXTRA_CFLAGS_LWIP += -DLWIP_DEBUG
endif

ifeq ($(ENABLE_ESPCONN), 1)
$(error LWIP2 does not support espconn_* functions. Make sure to set ENABLE_CUSTOM_LWIP to 0 or 1.)
endif

COMPONENT_SUBMODULES	:= lwip2
COMPONENT_INCDIRS		:= lwip2/glue-esp/include-esp lwip2/include

LWIP2_PATH := $(COMPONENT_PATH)/lwip2

# Make is pretty complex for LWIP2, and mucks about with output sections so build as a regular library
LWIP2_LIB			:= $(COMPONENT_NAME)
LWIP2_TARGET		:= $(COMPONENT_LIBDIR)/lib$(LWIP2_LIB).a
COMPONENT_TARGETS	:= $(LWIP2_TARGET)
EXTRA_LIBS			:= $(LWIP2_LIB)

$(COMPONENT_RULE)$(LWIP2_TARGET):
	$(Q) $(MAKE) -C $(LWIP2_PATH) -f Makefile.sming BUILD=$(COMPONENT_BUILD_DIR) \
				USER_LIBDIR=$(COMPONENT_LIBDIR)/ CFLAGS_EXTRA="$(EXTRA_CFLAGS_LWIP)" CC=$(CC) AR=$(AR) all
