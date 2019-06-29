COMPONENT_SRCFILES	:= $(ESP8266_COMPONENTS)/driver/SerialBuffer.cpp
COMPONENT_INCDIRS	+= $(ESP8266_COMPONENTS)/driver $(ESP8266_COMPONENTS)/driver/include

##@Tools

DEBUG_VARS	?= UARTID
UARTID		?= 0

TELNET_CMDLINE	= telnet localhost $$((10000 + $(UARTID)))

.PHONY: telnet
telnet: ##Run telnet to connect to a virtual serial port, specified by UARTID
ifeq ($(UNAME),Windows)
	start $(TELNET_CMDLINE)
else
	$(TELNET_CMDLINE) &
endif

