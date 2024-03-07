COMPONENT_SOC := esp* host
COMPONENT_DEPENDS := OtaNetwork

HWCONFIG := ota

CONFIG_VARS := RBOOT_TWO_ROMS 
RBOOT_TWO_ROMS := 1

# download urls, set appropriately
CONFIG_VARS += ROM_0_URL \
			   ROM_1_URL \
			   SPIFFS_URL
			    
ROM_0_URL := "http://192.168.7.5:80/rom0.bin"
ROM_1_URL := "http://192.168.7.5:80/rom1.bin"
SPIFFS_URL := "http://192.168.7.5:80/spiff_rom.bin"

APP_CFLAGS = -DROM_0_URL="\"$(ROM_0_URL)"\" \
             -DROM_1_URL="\"$(ROM_1_URL)"\" \
             -DSPIFFS_URL="\"$(SPIFFS_URL)"\"
             
ifneq ($(RBOOT_TWO_ROMS),)
	APP_CFLAGS += -DRBOOT_TWO_ROMS=$(RBOOT_TWO_ROMS)
endif