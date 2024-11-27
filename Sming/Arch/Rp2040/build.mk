##############
#
# Rp2040 Architecture build environment
#
##############

CPPFLAGS += \
	-DARCH_RP2040 \
	-DARDUINO_ARCH_RP2040 \
	-DPICO_NO_HARDWARE=0 \
	-DPICO_ON_DEVICE=1 \
	-DPICO_32BIT=1 \
	-mthumb \
	-nostdlib

ifeq ($(SMING_SOC),rp2350)
RP_VARIANT = rp2350
CPPFLAGS += \
	-mcpu=cortex-m33 \
	-march=armv8-m.main+fp+dsp \
	-mfloat-abi=softfp \
	-mcmse \
	-DPICO_RP2350
else
RP_VARIANT = rp2040
CPPFLAGS += \
	-mcpu=cortex-m0plus \
	-DPICO_RP2040
endif

CXXFLAGS += \
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
ifeq (,$(shell which $(CONFIG_TOOLPREFIX)-gcc))
$(error Toolchain not found, maybe set PICO_TOOLCHAIN_PATH)
endif
TOOLSPEC := $(CONFIG_TOOLPREFIX)-
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
GDB				:= gdb

# => Tools
MEMANALYZER = $(PYTHON) $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
