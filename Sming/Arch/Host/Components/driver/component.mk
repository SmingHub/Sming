
# => SMING

MODULES			+= $(ARCH_COMPONENTS)/driver
EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/driver/include $(ESP8266_COMPONENTS)/driver/include



# APP

EXTRA_INCDIR	+= $(ARCH_COMPONENTS)/driver/include $(ESP8266_COMPONENTS)/driver/include


Terminal = start telnet localhost $$((10000 + $1))

TERMINAL = $(call Terminal,$(COM_PORT))
KILL_TERM :=
