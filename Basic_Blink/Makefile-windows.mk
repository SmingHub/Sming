# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= c:/Espressif/ESP8266_SDK

# esptool path and port
SDK_TOOLS	 ?= c:/Espressif/utils
ESPTOOL		 ?= $(SDK_TOOLS)/esptool.exe
GEN_APPBIN   ?= $(SDK_TOOLS)/gen_appbin.exe
GEN_FLASHBIN ?= $(SDK_TOOLS)/gen_flashbin.exe
KILL_TERM    ?= $(shell taskkill /f /im termite.exe)

COM_PORT	 ?= COM3