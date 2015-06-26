# ESP8266 sdk package home directory
ESP_HOME ?= /usr/local/esp8266/esp-open-sdk

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

# Other tools mappings
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py
KILL_TERM    ?= pkill -9 -f /dev/cuaU0
GET_FILESIZE ?= stat -f "%-15z"

COM_PORT     ?= /dev/cuaU0

