COMPONENT_SRCFILES	:= $(ESP8266_COMPONENTS)/driver/SerialBuffer.cpp
COMPONENT_INCDIRS	+= $(ESP8266_COMPONENTS)/driver $(ESP8266_COMPONENTS)/driver/include

##@Tools

# Starting IP port number for uart servers
CACHE_VARS			+= HOST_UART_PORTBASE
HOST_UART_PORTBASE	?= 10000

# List of UART IDs to run servers for
CACHE_VARS			+= ENABLE_HOST_UARTID
ENABLE_HOST_UARTID	?=

# Flags to add when running emulator
HOST_UART_FLAGS		= $(foreach id,$(ENABLE_HOST_UARTID),--uart=$(id))

# $1 -> Uart ID
define RunHostTerminal
$(call DetachCommand,telnet localhost $$(($(HOST_UART_PORTBASE) + $1)))
endef
