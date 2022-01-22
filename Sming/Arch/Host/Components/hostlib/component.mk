EXTRA_LIBS		:= pthread

ifeq ($(UNAME),Windows)
	EXTRA_LIBS	+= wsock32
else
	EXTRA_LIBS	+= rt
endif

COMPONENT_DEPENDS := \
	driver \
	spi_flash

ifneq ($(DISABLE_WIFI),1)
COMPONENT_DEPENDS += \
	esp_wifi \
	lwip
endif

# Build a variant depending on network support
COMPONENT_RELINK_VARS	:= DISABLE_NETWORK

COMPONENT_INCDIRS		:= include
COMPONENT_SRCDIRS		:= .
COMPONENT_DOXYGEN_INPUT := include/hostlib

# Optional command line parameters passed to host application
CACHE_VARS				+= HOST_PARAMETERS
