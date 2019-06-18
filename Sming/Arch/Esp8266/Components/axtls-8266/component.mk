# => SMING

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




# => APP

# SSL support using axTLS
CONFIG_VARS += ENABLE_SSL SSL_DEBUG
ifeq ($(ENABLE_SSL),1)
	LIBAXTLS			:= axtls
	LIBS				+= $(LIBAXTLS)
	LIBAXTLS_DST		:= $(call UserLibPath,$(LIBAXTLS))
	AXTLS_BASE			:= $(ARCH_COMPONENTS)/axtls-8266/axtls-8266
	EXTRA_INCDIR		+= $(AXTLS_BASE)/.. $(AXTLS_BASE) $(AXTLS_BASE)/ssl $(AXTLS_BASE)/crypto
	AXTLS_FLAGS			:= -DLWIP_RAW=1 -DENABLE_SSL=1
	ifeq ($(SSL_DEBUG),1) # 
		AXTLS_FLAGS		+= -DSSL_DEBUG=1 -DDEBUG_TLS_MEM=1
	endif

	CUSTOM_TARGETS		+= $(LIBAXTLS_DST) include/ssl/private_key.h
	CFLAGS				+= $(AXTLS_FLAGS)

$(LIBAXTLS_DST):
	$(call MakeLibrary,$@,ENABLE_SSL=1)

include/ssl/private_key.h:
	$(vecho) "Generating unique certificate and key. This may take some time"
	$(Q) mkdir -p $(CURDIR)/include/ssl/
	AXDIR=$(CURDIR)/include/ssl/ $(ARCH_COMPONENTS)/axtls-8266/axtls-8266/tools/make_certs.sh
endif


