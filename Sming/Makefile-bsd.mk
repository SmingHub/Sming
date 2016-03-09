# ESP8266 sdk package home directory
ESP_HOME ?= /usr/local/esp8266/esp-open-sdk

# Default serial port
SERIAL_PORT     ?= /dev/cuaU0

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(ESP_HOME)/sdk
SDK_TOOLS	 ?= $(SDK_BASE)/tools

# Other tools mappings
ESPTOOL		 ?= $(ESP_HOME)/esptool/esptool.py
KILL_TERM    ?= pkill -9 -f "$(SERIAL_PORT) $(SERIAL_BAUDRATE_APP)" || exit 0
GET_FILESIZE ?= stat -f "%-15z"
TERMINAL     ?= python -m serial.tools.miniterm $(SERIAL_PORT) $(SERIAL_BAUDRATE_APP)
MEMANALYZER  ?= $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text