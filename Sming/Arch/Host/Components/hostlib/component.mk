EXTRA_LIBS		:= pthread

ifeq ($(UNAME),Windows)
	EXTRA_LIBS	+= wsock32
endif

COMPONENT_DEPENDS := esp_wifi lwip driver spi_flash

COMPONENT_INCDIRS		:= include
COMPONENT_SRCDIRS		:= .
COMPONENT_DOXYGEN_INPUT := include/hostlib

# LWIP service interval (milliseconds)
COMPONENT_RELINK_VARS	+= LWIP_SERVICE_INTERVAL
LWIP_SERVICE_INTERVAL	?= 2
COMPONENT_CXXFLAGS		+= -DLWIP_SERVICE_INTERVAL=$(LWIP_SERVICE_INTERVAL)

# Optional command line parameters passed to host application
CACHE_VARS				+= HOST_PARAMETERS
