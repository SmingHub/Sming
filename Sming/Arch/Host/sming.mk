# Makefile to build Sming framework under Host (Win32/Linux) environment

EXCLUDE_LIBRARIES := Adafruit_ILI9341 Adafruit_NeoPixel Adafruit_PCD8544 Adafruit_SSD1306 \
					ArduCAM CapacitiveSensor IR MCP23S17 RF24 SDCard TFT_ILI9163C WS2812

ESP8266_COMPONENTS	:= Arch/Esp8266/Components

#
MODULES			+= $(ARCH_COMPONENTS)/esp_hal
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp_hal/include $(ESP8266_COMPONENTS)/esp8266/include

#
MODULES			+= $(ARCH_COMPONENTS)/driver
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/driver/include $(ESP8266_COMPONENTS)/driver/include

#
MODULES			+= $(ARCH_COMPONENTS)/libc
MODULES			+= $(ARCH_COMPONENTS)/hostlib

#
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/spi_flash/include $(ESP8266_COMPONENTS)/spi_flash/include

#
MODULES			+= $(ARCH_COMPONENTS)/heap
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/heap/include

#
MODULES			+= $(ARCH_COMPONENTS)/gdbstub

#
MODULES			+= $(ARCH_COMPONENTS)/esp_wifi
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/esp_wifi/include

# => LWIP
ENABLE_CUSTOM_LWIP ?= 2
ifeq ($(ENABLE_CUSTOM_LWIP), 2)
	LWIP_BASE			:= $(ARCH_COMPONENTS)/lwip
	SUBMODULES			+= $(LWIP_BASE)/lwip
	EXTRA_INCDIR		+= $(LWIP_BASE) $(LWIP_BASE)/lwip/src/include
	LIBLWIP				:= lwip
	LIBS				+= $(LIBLWIP)
	ENABLE_LWIPDEBUG	?= 0
	ifeq ($(ENABLE_LWIPDEBUG), 1)
		CMAKE_OPTIONS	:= -DCMAKE_BUILD_TYPE=Debug
	else
		CMAKE_OPTIONS	:= -DCMAKE_BUILD_TYPE=Release
	endif
	LWIP_BUILD_DIR		:= $(BUILD_BASE)/$(LWIP_BASE)

	ifeq ($(UNAME),Windows)
		EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/win32/include
		CMAKE_OPTIONS	+= -G "MSYS Makefiles"
	else
		EXTRA_INCDIR 	+= $(LWIP_BASE)/lwip/contrib/ports/unix/port/include
	endif

	CLEAN				+= lwip-clean

$(call UserLibPath,lwip): $(LWIP_BUILD_DIR)/Makefile
	$(vecho) "Building $(notdir $@)..."
	$(Q) $(MAKE) -C $(LWIP_BUILD_DIR)

$(LWIP_BUILD_DIR)/Makefile: $(LWIP_BASE)/lwip/.submodule | $(LWIP_BUILD_DIR)
	$(Q) cd $(LWIP_BUILD_DIR); \
	$(CMAKE) -DUSER_LIBDIR="$(SMING_HOME)/$(USER_LIBDIR)" $(CMAKE_OPTIONS) $(SMING_HOME$)/$(LWIP_BASE)/$(UNAME)

$(LWIP_BUILD_DIR):
	mkdir -p $@

.PHONY: lwip-clean
lwip-clean:
	-$(Q) rm -rf $(LWIP_BUILD_DIR)
endif

#
SPIFFS_SMING	:= $(ESP8266_COMPONENTS)/spiffs
SPIFFS_BASE		:= $(COMPONENTS)/spiffs
SUBMODULES		+= $(SPIFFS_BASE)
MODULES			+= $(SPIFFS_SMING) $(SPIFFS_BASE)/src
CFLAGS			+= -D__WORDSIZE=32 # spiffy_host.h
EXTRA_INCDIR	+= $(SPIFFS_SMING) $(SPIFFS_BASE)/src
TOOLS			+= $(SPIFFY)
TOOLS_CLEAN		+= tools-clean-esp8266

$(SPIFFY):
	$(MAKE) SMING_ARCH=Esp8266 tools

.PHONY: tools-clean-esp8266
tools-clean-esp8266:
	$(MAKE) SMING_ARCH=Esp8266 tools-clean
	

# Not all libraries will compile (yet)
SMING_LIBRARIES	:= ArduinoJson
