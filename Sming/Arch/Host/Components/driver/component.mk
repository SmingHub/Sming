COMPONENT_SRCFILES := \
	$(ESP8266_COMPONENTS)/driver/SerialBuffer.cpp

COMPONENT_INCDIRS := \
	include \
	$(ESP8266_COMPONENTS)/driver \
	$(ESP8266_COMPONENTS)/driver/include

#
COMPONENT_VARS	+= USE_US_TIMER
USE_US_TIMER	?= 1
ifeq ($(USE_US_TIMER),1)
GLOBAL_CFLAGS	+= -DUSE_US_TIMER
endif

##@Tools

# Starting IP port number for uart servers
CACHE_VARS			+= HOST_UART_PORTBASE
HOST_UART_PORTBASE	?= 10000

# List of UART IDs to run servers for
CACHE_VARS			+= ENABLE_HOST_UARTID
ENABLE_HOST_UARTID	?=

# Options to add when running emulator
CACHE_VARS			+= HOST_UART_OPTIONS
HOST_UART_OPTIONS	?= $(addprefix --uart=,$(ENABLE_HOST_UARTID))
CLI_TARGET_OPTIONS += $(HOST_UART_OPTIONS)

# $1 -> Uart ID
define RunHostTerminal
$(call DetachCommand,telnet localhost $$(($(HOST_UART_PORTBASE) + $1)))
endef
