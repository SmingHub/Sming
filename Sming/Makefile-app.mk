#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

### Defaults ###

CONFIG_VARS		+= SERVER_OTA_PORT
SERVER_OTA_PORT ?= 9999

## COM port parameters
# Default COM port speed (generic)
COM_SPEED ?= 115200

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

# Default COM port speed (used in code)
COM_SPEED_SERIAL  ?= $(COM_SPEED)

include $(SMING_HOME)/build.mk

# name for the target project
TARGET			:= app

ARCH_BASE		:= $(SMING_HOME)/$(ARCH_BASE)
COMPONENTS		:= $(SMING_HOME)/$(COMPONENTS)

CONFIG_VARS		+= CURDIR MAKE_VERSION SHELL

#
CONFIG_VARS		+= SPIFF_FILES
SPIFF_FILES		?= files

# Firmware memory layout info files
FW_MEMINFO_NEW		:= $(FW_BASE)/fwMeminfo.new
FW_MEMINFO_OLD		:= $(FW_BASE)/fwMeminfo.old
FW_MEMINFO_SAVED	:= out/fwMeminfo

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES      ?= app     # default to app if not set by user
EXTRA_INCDIR ?= include # default to include if not set by user

SMING_INCDIR := System/include Wiring Core

EXTRA_INCDIR += $(SMING_HOME) $(ARCH_BASE) $(ARCH_CORE) $(ARCH_SYS)/include \
				$(ARCH_COMPONENTS) $(COMPONENTS) \
				$(addprefix $(SMING_HOME)/,$(SMING_INCDIR))

# we will use global WiFi settings from Eclipse Environment Variables, if possible
CONFIG_VARS	+= WIFI_SSID WIFI_PWD
WIFI_SSID	?= ""
WIFI_PWD	?= ""
ifneq ($(WIFI_SSID), "")
	CFLAGS	+= -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
ifneq ($(WIFI_PWD), "")
	CFLAGS	+= -DWIFI_PWD=\"$(WIFI_PWD)\"
endif

#
CONFIG_VARS	+= DISABLE_SPIFFS
ifeq ($(DISABLE_SPIFFS), 1)
	CFLAGS	+= -DDISABLE_SPIFFS=1
endif

# => Serial
CONFIG_VARS	+= COM_SPEED_SERIAL
CFLAGS		+= -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL)

include $(ARCH_BASE)/app.mk

#
.PHONY: kill_term
kill_term:
	$(vecho) "Killing Terminal to free $(COM_PORT)"
	-$(Q) $(KILL_TERM)

##@Tools

.PHONY: terminal
terminal: kill_term ##Open the serial terminal
	$(TERMINAL)

.PHONY: gdb
gdb: kill_term ##Run the debugger console
	$(GDB)

# Stack trace decoder
CONFIG_VARS += TRACE
TRACE ?=
.PHONY: decode-stacktrace
decode-stacktrace: ##Open the stack trace decoder ready to paste dump text. Alteratively, use `make decode-stacktrace TRACE=/path/to/crash.stack`
	$(Q) if [ -z "$(TRACE)" ]; then \
		echo "Decode stack trace: Paste stack trace here"; \
	fi
	$(Q) python $(ARCH_TOOLS)/decode-stacktrace.py $(TARGET_OUT_0) $(TRACE)

##@Testing

.PHONY: otaserver
otaserver: all ##Launch a simple python HTTP server for testing OTA updates
	$(vecho) "Starting OTA server for TESTING"
	$(Q) cd $(FW_BASE) && python -m SimpleHTTPServer $(SERVER_OTA_PORT)

##@Help

.PHONY: list-config
list-config: ##Print the contents of internal build variables
	$(call ListConfig)

# => Help
.PHONY: help
help: ##Show this help summary
	$(call PrintHelp)
