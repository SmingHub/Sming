COMPONENT_SRCDIRS := \
	Core $(call ListAllSubDirs,$(COMPONENT_PATH)/Core) \
	Platform \
	System \
	Wiring \
	Services/HexDump \
	Services/Yeelight

COMPONENT_INCDIRS := \
	Components \
	System/include \
	Wiring Core \
	.

COMPONENT_DEPENDS := \
	sming-arch \
	spiffs \
	http-parser \
	libb64 \
	ws_parser \
	mqtt-codec \
	libyuarel \
	terminal

# => SSL
COMPONENT_VARS 			:= ENABLE_SSL
ifeq ($(ENABLE_SSL),1)
	SMING_FEATURES		:= SSL
	GLOBAL_CFLAGS		+= -DENABLE_SSL=1
else
	SMING_FEATURES		:= none
	COMPONENT_SRCDIRS	:= $(filter-out %/Ssl,$(COMPONENT_SRCDIRS))
endif

# Prints SSL status when App gets built
CUSTOM_TARGETS			+= check-ssl
.PHONY:check-ssl
check-ssl:
ifeq ($(ENABLE_SSL),1)
	$(info + SSL support is enabled)
else
	$(warning ! SSL support is not enabled. To enable it type: 'make clean; make ENABLE_SSL=1')
endif

# => Disable CommandExecutor functionality if not used and save some ROM and RAM
COMPONENT_VARS			+= ENABLE_CMD_EXECUTOR
ENABLE_CMD_EXECUTOR		?= 1
ifeq ($(ENABLE_CMD_EXECUTOR),1)
COMPONENT_SRCDIRS		+= Services/CommandProcessing
endif
GLOBAL_CFLAGS			+= -DENABLE_CMD_EXECUTOR=$(ENABLE_CMD_EXECUTOR)

# => MQTT
# Flags for compatability with old versions (most of them should disappear with the next major release)
COMPONENT_VARS			+= MQTT_NO_COMPAT
ifeq ($(MQTT_NO_COMPAT),1)
	GLOBAL_CFLAGS		+= -DMQTT_NO_COMPAT=1
endif

# WiFi settings may be provide via Environment variables
CONFIG_VARS				+= WIFI_SSID WIFI_PWD
ifdef WIFI_SSID
	APP_CFLAGS			+= -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
ifdef WIFI_PWD
	APP_CFLAGS			+= -DWIFI_PWD=\"$(WIFI_PWD)\"
endif

# => WPS
COMPONENT_VARS			+= ENABLE_WPS
ifeq ($(ENABLE_WPS), 1)
	GLOBAL_CFLAGS		+= -DENABLE_WPS=1
endif

# => Smart Config
COMPONENT_VARS			+= ENABLE_SMART_CONFIG
ifeq ($(ENABLE_SMART_CONFIG),1)
	GLOBAL_CFLAGS		+= -DENABLE_SMART_CONFIG=1
endif

# => LOCALE
COMPONENT_VARS			+= LOCALE
ifdef LOCALE
	GLOBAL_CFLAGS		+= -DLOCALE=$(LOCALE)
endif

# => Multipart Parser
COMPONENT_VARS 			+= ENABLE_HTTP_SERVER_MULTIPART
ifeq ($(ENABLE_HTTP_SERVER_MULTIPART),1)
	SMING_FEATURES		+= HTTP_SERVER_MULTIPART
	GLOBAL_CFLAGS		+= -DENABLE_HTTP_SERVER_MULTIPART=1
	COMPONENT_DEPENDS	+= MultipartParser
endif

### Debug output parameters

# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
CONFIG_VARS				+= DEBUG_PRINT_FILENAME_AND_LINE
DEBUG_PRINT_FILENAME_AND_LINE ?= 0
GLOBAL_CFLAGS			+= -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)
# When rules are created make will see '$*' so substitute the filename
GLOBAL_CFLAGS				+= -DCUST_FILE_BASE=$$*

# Default debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0
CONFIG_VARS				+= DEBUG_VERBOSE_LEVEL
DEBUG_VERBOSE_LEVEL		?= 2
GLOBAL_CFLAGS			+= -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL)

CONFIG_VARS			+= ENABLE_GDB
ifeq ($(ENABLE_GDB), 1)
	GLOBAL_CFLAGS	+= -ggdb -DENABLE_GDB=1
endif

# Default COM port speed (generic)
CACHE_VARS			+= COM_SPEED
COM_SPEED			?= 115200

# Default COM port speed used in code
CONFIG_VARS			+= COM_SPEED_SERIAL
COM_SPEED_SERIAL	?= $(COM_SPEED)
APP_CFLAGS			+= -DCOM_SPEED_SERIAL=$(COM_SPEED_SERIAL)

