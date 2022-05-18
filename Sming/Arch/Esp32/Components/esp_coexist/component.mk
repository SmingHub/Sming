COMPONENT_DEPENDS := esp32

COMPONENT_SRCDIRS := src

COMPONENT_PREREQUISITES := $(COMPONENT_PATH)/src/coex_printf.c

$(COMPONENT_PATH)/src/coex_printf.c: $(IDF_PATH)/components/esp_wifi/src/lib_printf.c
	$(Q) cp $< $@