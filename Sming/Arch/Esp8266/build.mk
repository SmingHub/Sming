##############
#
# Esp8266 Architecture build environment
#
##############

CFLAGS		+= -DARCH_ESP8266
CXXFLAGS	+= -fno-rtti -fno-exceptions 

ifndef ESP_HOME
$(error ESP_HOME variable is not set to a valid directory.)
endif

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk

CONFIG_VARS	+= ESP_HOME
ESP_HOME	:= $(call FixPath,$(ESP_HOME))
export ESP_HOME

XTENSA_TOOLS_ROOT	:= $(ESP_HOME)/xtensa-lx106-elf/bin
export PATH			:= $(XTENSA_TOOLS_ROOT):$(PATH)
CONFIG_TOOLPREFIX	:= xtensa-lx106-elf-
TOOLSPEC			:= $(XTENSA_TOOLS_ROOT)/$(CONFIG_TOOLPREFIX)

# select which tools to use as assembler, compiler, librarian and linker
AS		:= $(TOOLSPEC)gcc
CC		:= $(TOOLSPEC)gcc
CXX		:= $(TOOLSPEC)g++
AR		:= $(TOOLSPEC)ar
LD		:= $(TOOLSPEC)gcc
OBJCOPY := $(TOOLSPEC)objcopy
OBJDUMP := $(TOOLSPEC)objdump
GDB		:= $(TOOLSPEC)gdb

CFLAGS_COMMON	+= -nostdlib -mlongcalls -mtext-section-literals
CFLAGS			+= -D__ets__ -DICACHE_FLASH -DUSE_OPTIMIZE_PRINTF -DESP8266=1

CONFIG_VARS += MFORCE32
MFORCE32 := $(shell $(CC) --help=target | grep mforce-l32)
ifneq ($(MFORCE32),)
	CFLAGS += -DMFORCE32 -mforce-l32
endif

# => 'Internal' SDK - for SDK Version 3+ as submodule in Sming repository
# SDK_BASE just needs to point into our repo as it's overridden with the correct submodule path
# This provides backward-compatiblity, so $(SMING)/third-party/ESP8266_NONOS_SDK) still works
CONFIG_VARS += SDK_BASE SDK_INTERNAL
SDK_BASE := $(call FixPath,$(abspath $(SDK_BASE)))
ifneq (,$(findstring $(SMING_HOME),$(SDK_BASE)))
	SDK_COMPONENT	:= $(ARCH_COMPONENTS)/Sdk/ESP8266_NONOS_SDK
	SDK_BASE		:= $(SMING_HOME)/$(SDK_COMPONENT)
	CFLAGS			+= -DSDK_INTERNAL
	SDK_INTERNAL = 1
else
	SDK_INTERNAL = 0
endif

# various paths from the SDK used in this project
SDK_LIBDIR	:= $(SDK_BASE)/lib
SDK_INCDIR	:= $(SDK_BASE)/include

# => Tools
ESPTOOL2	= $(ARCH_TOOLS)/esptool2/esptool2$(TOOL_EXT)
SPIFFY		= $(ARCH_TOOLS)/spiffy/spiffy$(TOOL_EXT)
MEMANALYZER = python $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
