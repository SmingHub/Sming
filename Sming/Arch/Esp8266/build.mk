##############
#
# Esp8266 Architecture build environment
#
##############

CPPFLAGS			+= -DARCH_ESP8266
CXXFLAGS			+= -fno-threadsafe-statics

## ESP_HOME sets the path where ESP tools and SDK are located.
DEBUG_VARS			+= ESP_HOME

ifeq ($(UNAME),Windows)
ESP_HOME			?= c:/tools/esp-quick-toolchain
else
ESP_HOME			?= /opt/esp-quick-toolchain
endif
export ESP_HOME		:= $(call FixPath,$(ESP_HOME))

XTENSA_TOOLS_ROOT	:= $(ESP_HOME)/xtensa-lx106-elf/bin
export PATH			:= $(XTENSA_TOOLS_ROOT):$(PATH)
CONFIG_TOOLPREFIX	:= xtensa-lx106-elf-
TOOLSPEC			:= $(XTENSA_TOOLS_ROOT)/$(CONFIG_TOOLPREFIX)

# select which tools to use as assembler, compiler, librarian and linker
AS				:= $(TOOLSPEC)gcc
CC				:= $(TOOLSPEC)gcc
CXX				:= $(TOOLSPEC)g++
AR				:= $(TOOLSPEC)ar
LD				:= $(TOOLSPEC)gcc
OBJCOPY			 	:= $(TOOLSPEC)objcopy
OBJDUMP			 	:= $(TOOLSPEC)objdump
NM				:= $(TOOLSPEC)nm
GDB				:= $(TOOLSPEC)gdb

CPPFLAGS += \
	-nostdlib \
	-mlongcalls \
	-mtext-section-literals

CPPFLAGS += \
	-D__ets__ \
	-DICACHE_FLASH \
	-DUSE_OPTIMIZE_PRINTF \
	-DESP8266 \
	-D__ESP8266_EX__

ifeq (,$(wildcard $(XTENSA_TOOLS_ROOT)))
$(error ESP_HOME not set correctly: "$(ESP_HOME)")
endif

# => Tools
MEMANALYZER = $(PYTHON) $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
