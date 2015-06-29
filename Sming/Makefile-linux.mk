# ESP8266 sdk package home directory
ESP_HOME ?= /opt/esp-open-sdk

# Default COM port
COM_PORT     ?= /dev/ttyUSB0

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

# Other tools mappings
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py
KILL_TERM    ?= pkill screen
GET_FILESIZE ?= stat --printf="%s"