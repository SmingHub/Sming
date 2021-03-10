#### overridable rBoot options ####

## use rboot build mode
RBOOT_ENABLED := 1

## Use standard hardware config with two ROM slots and two SPIFFS partitions
ifeq ($(SMING_ARCH),Esp8266)
HWCONFIG := basic_rboot
else
HWCONFIG := spiffs
endif
