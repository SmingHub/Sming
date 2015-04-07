# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= /opt/esp-open-sdk/xtensa-lx106-elf/bin	

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= /opt/esp-open-sdk/sdk

# esptool path and port
SDK_TOOLS	 ?= c:/Espressif/utils
ESPTOOL		 ?= $(SDK_TOOLS)/esptool.py
GEN_APPBIN   ?= $(SDK_TOOLS)/gen_appbin.py
GEN_FLASHBIN ?= $(SDK_TOOLS)/gen_flashbin.py
KILL_TERM    ?= $(shell pkill screen)

COM_PORT     ?= /dev/tty.usbserial