##############
#
# Rp2040 Architecture build environment
#
##############

CPPFLAGS += \
	-DARCH_RP2040 \
	-march=armv6-m \
	-mcpu=cortex-m0plus \
	-mthumb

CXXFLAGS += \
	-fno-exceptions \
	-fno-threadsafe-statics \
	-fno-use-cxa-atexit

CONFIG_TOOLPREFIX := arm-none-eabi

# This will differ based on platform
DEBUG_VARS += PICO_TOOLCHAIN_PATH
ifdef PICO_TOOLCHAIN_PATH
export PICO_TOOLCHAIN_PATH := $(call FixPath,$(PICO_TOOLCHAIN_PATH))
TOOLSPEC := $(PICO_TOOLCHAIN_PATH)/bin/$(CONFIG_TOOLPREFIX)-
ifeq (,$(wildcard $(PICO_TOOLCHAIN_PATH)/$(CONFIG_TOOLPREFIX)))
$(error PICO_TOOLCHAIN_PATH not set correctly: $(PICO_TOOLCHAIN_PATH))
endif
else
PICO_TOOLCHAIN_PATH := $(shell which $(CONFIG_TOOLPREFIX)-gcc)
ifeq (,$(PICO_TOOLCHAIN_PATH))
$(error Toolchain not found, maybe set PICO_TOOLCHAIN_PATH)
endif
TOOLSPEC := $(dir $(PICO_TOOLCHAIN_PATH))$(CONFIG_TOOLPREFIX)-
endif

# select which tools to use as assembler, compiler, librarian and linker
AS				:= $(TOOLSPEC)gcc
CC				:= $(TOOLSPEC)gcc
CXX				:= $(TOOLSPEC)g++
AR				:= $(TOOLSPEC)ar
LD				:= $(TOOLSPEC)gcc
NM				:= $(TOOLSPEC)nm
OBJCOPY		 	:= $(TOOLSPEC)objcopy
OBJDUMP		 	:= $(TOOLSPEC)objdump
GDB				:= $(TOOLSPEC)gdb

CPPFLAGS += \
	-nostdlib

# => Tools
MEMANALYZER = $(PYTHON) $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
