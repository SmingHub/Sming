EXTRA_LIBS		:= pthread

ifeq ($(UNAME),Windows)
	EXTRA_LIBS	+= wsock32
endif

COMPONENT_DEPENDS := esp_wifi lwip driver spi_flash
