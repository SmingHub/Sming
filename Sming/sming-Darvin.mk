#	Darvin.mk
#
#==============================================================================
# 	default directories (make compatible format /<drive>/<absolute path>)
#------------------------------------------------------------------------------
# 	ESP8266 SDK package home directory
ESP_HOME 		?= /opt/esp-open-sdk

#==============================================================================
# 	default serial settings
#------------------------------------------------------------------------------
COM_PORT		?= /dev/tty.usbserial

#==============================================================================
# 	tool settings
#------------------------------------------------------------------------------
ESPTOOL		 	?= $(ESP_HOME)/esptool/esptool.py
GET_FILESIZE 	?= stat -L -f%z
MEMANALYZER  	?= $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text

PAUSE5			?= 

#==============================================================================
# 	serial terminal settings
#------------------------------------------------------------------------------
TERMINAL    	?= python -m serial.tools.miniterm $(COM_PORT) $(COM_SPEED_SERIAL) $(COM_OPTS)
KILL_TERM   	?= pkill -9 -f "$(COM_PORT) $(COM_SPEED_SERIAL)" || exit 0
