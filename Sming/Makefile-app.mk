#############################################################
#
# Created by Espressif
# UDK modifications by CHERTS <sleuthhound@gmail.com>
# Cross platform compatability by kireevco <dmitry@kireev.co>
#
#############################################################

### Defaults ###

SERVER_OTA_PORT ?= 9999

## COM port parameters
# Default COM port speed (generic)
COM_SPEED ?= 115200

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

# Default COM port speed (used in code)
COM_SPEED_SERIAL  ?= $(COM_SPEED)

include $(SMING_HOME)/build.mk

PLATFORM_BASE	:= $(SMING_HOME)/$(PLATFORM_BASE)
THIRD_PARTY_DIR	:= $(SMING_HOME)/$(THIRD_PARTY_DIR)

BUILD_BASE		:= out/build
FW_BASE			:= out/firmware

CURRENT_DIR		:= $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

SPIFF_FILES ?= files

#Firmware memory layout info files
FW_MEMINFO_NEW = $(FW_BASE)/fwMeminfo.new
FW_MEMINFO_OLD = $(FW_BASE)/fwMeminfo.old
FW_MEMINFO_SAVED = out/fwMeminfo

# name for the target project
TARGET = app

LIBSMING = sming
SMING_FEATURES = none
ifeq ($(ENABLE_SSL),1)
	LIBSMING = smingssl
	SMING_FEATURES = SSL
endif

# which modules (subdirectories) of the project to include in compiling
# define your custom directories in the project's own Makefile before including this one
MODULES      ?= app     # default to app if not set by user
EXTRA_INCDIR ?= include # default to include if not set by user

SMING_INCDIR := System/include Wiring Libraries SmingCore Platform/Common \
				Libraries/Adafruit_GFX Libraries/Adafruit_Sensor

EXTRA_INCDIR += $(SMING_HOME) $(addprefix $(SMING_HOME)/,$(SMING_INCDIR)) \
				$(PLATFORM_BASE) $(PLATFORM_SYS)/include $(PLATFORM_CORE) \
				$(THIRD_PARTY_DIR)

# we will use global WiFi settings from Eclipse Environment Variables, if possible
WIFI_SSID ?= ""
WIFI_PWD ?= ""
ifneq ($(WIFI_SSID), "")
	CFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
ifneq ($(WIFI_PWD), "")
	CFLAGS += -DWIFI_PWD=\"$(WIFI_PWD)\"
endif
ifeq ($(DISABLE_SPIFFS), 1)
	CFLAGS += -DDISABLE_SPIFFS=1
endif

# => Serial
CFLAGS += -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL) $(USER_CFLAGS)

include $(PLATFORM_BASE)/app.mk

