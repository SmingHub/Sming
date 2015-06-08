# ESP8266 sdk package home directory
ESP_HOME ?= /usr/local/esp-open-sdk

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

# Other tools mappings
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py
GEN_APPBIN   := PATH="$(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)" && $(SDK_TOOLS)/gen_appbin.py
GEN_FLASHBIN := PATH="$(ESP_HOME)/xtensa-lx106-elf/bin:$(PATH)" && $(SDK_TOOLS)/gen_flashbin.py
KILL_TERM    ?= pkill -9 -f /dev/cuaU0
GET_FILESIZE ?= stat -f "%-15z"

COM_PORT     ?= /dev/cuaU0

