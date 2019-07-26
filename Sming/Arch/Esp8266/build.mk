##############
#
# Esp8266 Architecture build environment
#
##############

CFLAGS				+= -DARCH_ESP8266
CXXFLAGS			+= -fno-rtti -fno-exceptions 

## ESP_HOME sets the path where ESP tools and SDK are located.
DEBUG_VARS			+= ESP_HOME

ifeq ($(UNAME),Windows)
ESP_HOME			?= c:/Espressif
else ifeq ($(UNAME),FreeBSD)
ESP_HOME			?= /usr/local/esp8266/esp-open-sdk
else
ESP_HOME			?= /opt/esp-open-sdk
endif
export ESP_HOME		:= $(call FixPath,$(ESP_HOME))

XTENSA_TOOLS_ROOT	:= $(ESP_HOME)/xtensa-lx106-elf/bin
export PATH			:= $(XTENSA_TOOLS_ROOT):$(PATH)
CONFIG_TOOLPREFIX	:= xtensa-lx106-elf-
TOOLSPEC			:= $(XTENSA_TOOLS_ROOT)/$(CONFIG_TOOLPREFIX)

# select which tools to use as assembler, compiler, librarian and linker
DEBUG_VARS			+= GDB
AS					:= $(TOOLSPEC)gcc
CC					:= $(TOOLSPEC)gcc
CXX					:= $(TOOLSPEC)g++
AR					:= $(TOOLSPEC)ar
LD					:= $(TOOLSPEC)gcc
OBJCOPY			 	:= $(TOOLSPEC)objcopy
OBJDUMP			 	:= $(TOOLSPEC)objdump
GDB					:= $(TOOLSPEC)gdb

CFLAGS_COMMON += \
	-nostdlib \
	-mlongcalls \
	-mtext-section-literals

CFLAGS += \
	-D__ets__ \
	-DICACHE_FLASH \
	-DUSE_OPTIMIZE_PRINTF \
	-DESP8266

ifeq (,$(wildcard $(XTENSA_TOOLS_ROOT)))
$(error ESP_HOME not set correctly: "$(ESP_HOME)")
endif

# Both flash and peripheral memories must be accessed on 4-byte word boundaries,
# otherwise behaviour can be unpredictable or cause a memory exception.
# The -mforce-l32 compiler option generates code to deal with reads of
# mis-aligned memory accesses. It is not a standard feature of GNU compilers,
# however, and is not always available.
DEBUG_VARS			+= MFORCE32
MFORCE32 := $(firstword $(shell $(CC) --help=target | grep mforce-l32))
ifneq ($(MFORCE32),)
	CFLAGS			+= -DMFORCE32 -mforce-l32
endif

# => Tools
MEMANALYZER = python $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
