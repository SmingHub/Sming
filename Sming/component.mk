#
# Definition for the sming library
#

SMING_INCDIR := System/include Wiring Core

EXTRA_INCDIR += $(SMING_HOME) $(ARCH_BASE) $(ARCH_CORE) $(ARCH_SYS)/include \
				$(ARCH_COMPONENTS) $(COMPONENTS) \
				$(addprefix $(SMING_HOME)/,$(SMING_INCDIR))

# Services
COMPONENT_SRCDIRS	+= $(wildcard Services/*/)

# => http-parser
SUBMODULES		+= $(COMPONENTS)/http-parser
MODULES			+= $(COMPONENTS)/http-parser

# => libb64
MODULES			+= $(COMPONENTS)/libb64

# => websocket-parser
SUBMODULES		+= $(COMPONENTS)/ws_parser
MODULES			+= $(COMPONENTS)/ws_parser

# => mqtt-codec
SUBMODULES		+= $(COMPONENTS)/mqtt-codec
MODULES			+= $(COMPONENTS)/mqtt-codec/src
EXTRA_INCDIR	+= $(COMPONENTS)/mqtt-codec/src


# => yuarel
SUBMODULES		+= $(COMPONENTS)/libyuarel
MODULES			+= $(COMPONENTS)/libyuarel


# From build.mk

### Debug output parameters
# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
CONFIG_VARS += DEBUG_PRINT_FILENAME_AND_LINE
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

# Default debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0
CONFIG_VARS += DEBUG_VERBOSE_LEVEL
DEBUG_VERBOSE_LEVEL ?= 2

#Append debug options
CFLAGS += -DCUST_FILE_BASE=$$* -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)


# Disable CommandExecutor functionality if not used and save some ROM and RAM
CONFIG_VARS += ENABLE_CMD_EXECUTOR
ENABLE_CMD_EXECUTOR ?= 1
CFLAGS += -DENABLE_CMD_EXECUTOR=$(ENABLE_CMD_EXECUTOR)


CONFIG_VARS += ENABLE_GDB
ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -ggdb -DENABLE_GDB=1
endif

# => LOCALE
ifdef LOCALE
	CFLAGS += -DLOCALE=$(LOCALE)
endif

# => Main Sming library
CONFIG_VARS += ENABLE_SSL
ifeq ($(ENABLE_SSL),1)
	LIBSMING		= smingssl
	SMING_FEATURES	= SSL
else
	LIBSMING		= sming
	SMING_FEATURES	= none
endif
LIBSMING_DST 		= $(call UserLibPath,$(LIBSMING))

# => MQTT
# Flags for compatability with old versions (most of them should disappear with the next major release)
CONFIG_VARS += MQTT_NO_COMPAT
ifeq ($(MQTT_NO_COMPAT),1)
	CFLAGS	+= -DMQTT_NO_COMPAT=1
endif



# From Makefile-app.mk

## COM port parameters
# Default COM port speed (generic)
COM_SPEED ?= 115200

# Default COM port speed (used in code)
COM_SPEED_SERIAL  ?= $(COM_SPEED)

# => Serial
CONFIG_VARS	+= COM_SPEED_SERIAL
CFLAGS		+= -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL)

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
