## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

#Add your source directories here separated by space
MODULES = app

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
#ESP_HOME = /opt/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

# MacOS / Linux
# SMING_HOME = /opt/sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

# MacOS / Linux:
# COM_PORT = /dev/tty.usbserial

DISABLE_SPIFFS = 1

# Com port speed
# COM_SPEED	= 115200
ENABLE_GDB=1

export ENABLE_GDB

ENABLE_GDB_CONSOLE ?= 1

ifeq ($(ENABLE_GDB_CONSOLE), 1)
	USER_CFLAGS += -DGDBSTUB_ENABLE_SYSCALL
endif

ifeq ($(ENABLE_GDB_CONSOLE), 1)
all:
	$(warning WARNING! Enabling the GDB console may interfere with visual debuggers, like eclipse)
	$(warning If required, please build with `make ENABLE_GDB_CONSOLE=0`)
endif

export USER_CFLAGS
