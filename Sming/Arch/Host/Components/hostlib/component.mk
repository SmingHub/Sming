EXTRA_LIBS		:= pthread

ifeq ($(UNAME),Windows)
	EXTRA_LIBS	+= wsock32
endif

COMPONENT_DEPENDS := esp_wifi lwip driver spi_flash

# LWIP service interval (milliseconds)
COMPONENT_VARS			+= LWIP_SERVICE_INTERVAL
LWIP_SERVICE_INTERVAL	?= 2
COMPONENT_CXXFLAGS		+= -DLWIP_SERVICE_INTERVAL=$(LWIP_SERVICE_INTERVAL)
