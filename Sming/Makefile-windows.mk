# ESP8266 sdk package home directory
ESP_HOME ?= c:/Espressif

# Default COM port
COM_PORT	 ?= COM3

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS	 ?= $(ESP_HOME)/utils

# Other tools mappings
ESPTOOL		 ?= $(SDK_TOOLS)/esptool/esptool.py
KILL_TERM    ?= taskkill.exe -f -im Terminal.exe || exit 0
GET_FILESIZE ?= stat --printf="%s"
TERMINAL     ?= $(SDK_TOOLS)/Terminal.exe $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)
