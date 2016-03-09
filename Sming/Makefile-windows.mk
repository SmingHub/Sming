# ESP8266 sdk package home directory
ESP_HOME ?= c:/Espressif

# Default serial port
SERIAL_PORT	 ?= COM3

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/ESP8266_SDK
SDK_TOOLS	 ?= $(ESP_HOME)/utils

# Other tools mappings
ESPTOOL		 ?= $(SDK_TOOLS)/esptool.exe
KILL_TERM    ?= taskkill.exe -f -im Terminal.exe || exit 0
GET_FILESIZE ?= stat --printf="%s"
TERMINAL     ?= $(SDK_TOOLS)/Terminal.exe $(SERIAL_PORT) $(SERIAL_BAUDRATE_APP)
MEMANALYZER  ?= $(SDK_TOOLS)/memanalyzer.exe $(OBJDUMP).exe