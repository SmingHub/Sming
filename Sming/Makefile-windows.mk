# ESP8266 sdk package home directory
ESP_HOME ?= c:/Espressif

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS	 ?= $(ESP_HOME)/utils

# Other tools mappings
ESPTOOL		 ?= $(SDK_TOOLS)/esptool.exe
GEN_APPBIN   := PATH="$(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)" && $(SDK_TOOLS)/gen_appbin.exe
GEN_APPBIN   := PATH="$(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)" && $(SDK_TOOLS)/gen_flashbin.exe

KILL_TERM    ?= $(shell taskkill /f /im termite.exe)
GET_FILESIZE ?= stat --printf="%s"

COM_PORT	 ?= COM3