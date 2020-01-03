COMPONENT_SUBMODULES	:= axtls-8266

COMPONENT_SRCDIRS := \
	axtls-8266/crypto \
	axtls-8266/ssl

ifneq ($(SMING_ARCH),Host)
COMPONENT_SRCDIRS += \
	axtls-8266/replacements
endif

COMPONENT_INCDIRS := .

EXTRA_INCDIR := \
	axtls-8266 \
	axtls-8266/ssl \
	axtls-8266/crypto

GLOBAL_CFLAGS			+= -DLWIP_RAW=1
COMPONENT_CFLAGS		:= -DWITH_PGM_READ_HELPER=1 -DAXTLS_BUILD
