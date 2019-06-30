# Some libraries won't build (and others won't work) on the Host
#
# These are the libraries which won't build. We don't need to explicitly exclude them any
# more because only those libraries specified by ARDUINO_LIBRARIES are built for a project.
#
# This list is kept for reference purposes if it is desirable, for example, to build all
# the libraries for testing or CI purposes. However, a better way to achieve that is to ensure
# that all libraries are referenced by at least one sample or test application.
#
# EXCLUDE_LIBRARIES := Adafruit_ILI9341 Adafruit_NeoPixel Adafruit_PCD8544 Adafruit_SSD1306 \
#					ArduCAM CapacitiveSensor IR MCP23S17 RF24 SDCard TFT_ILI9163C WS2812

# Eventually this will go, but for now we use some Esp8266 code
ESP8266_COMPONENTS := $(SMING_HOME)/Arch/Esp8266/Components
export ESP8266_COMPONENTS

COMPONENT_SRCDIRS := \
	$(ARCH_CORE) $(call ListAllSubDirs,$(ARCH_CORE)) \
	$(ARCH_SYS) \
	$(ARCH_BASE)/Platform

COMPONENT_INCDIRS := \
	$(ARCH_BASE) \
	$(ARCH_CORE) \
	$(ARCH_SYS)/include \
	$(ARCH_COMPONENTS)

COMPONENT_DEPENDS := \
	driver \
	esp_hal \
	esp_wifi \
	gdbstub \
	heap \
	hostlib \
	libc \
	lwip \
	spi_flash \
	vflash
