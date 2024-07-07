##############
#
# Esp32 Architecture build environment
#
##############

ifeq (,$(IDF_PATH))
$(error IDF_PATH not set correctly: "$(IDF_PATH)")
endif

export IDF_PATH := $(call FixPath,$(IDF_PATH))

# Extract IDF version
ifndef IDF_VER
# e.g. v5.2-beta1-265-g405b8b5512 or v5.0.5-173-g9d6770dfbb
IDF_VER := $(shell (cd $$IDF_PATH && git describe --always --tags --dirty) | cut -c 1-31)
endif
# Now just vmajor.minor
IDF_VERSION := $(subst ., ,$(firstword $(subst -, ,$(IDF_VER))))
IDF_VERSION := $(firstword $(IDF_VERSION)).$(word 2,$(IDF_VERSION))

# Use default paths from standard install scripts.
DEBUG_VARS += IDF_TOOLS_PATH
ifeq ($(UNAME),Windows)
IDF_TOOLS_PATH ?= C:/tools/esp32
else
IDF_TOOLS_PATH ?= /opt/esp32
endif

export IDF_TOOLS_PATH := $(call FixPath,$(IDF_TOOLS_PATH))

# ESP_VARIANT used internally for ESP32 only
ESP_VARIANT := $(SMING_SOC)
export ESP_VARIANT

IDF_TOOL_INFO := $(shell $(PYTHON) $(ARCH_TOOLS)/idf_tools.py $(SMING_SOC))
ESP32_GCC_PATH := $(word 1,$(IDF_TOOL_INFO))
ESP32_GDB_PATH := $(word 2,$(IDF_TOOL_INFO))
ESP32_COMPILER_PATH := $(IDF_TOOLS_PATH)/tools/$(ESP32_GCC_PATH)
ifneq (,$(filter xtensa%,$(ESP32_GCC_PATH)))
ESP32_COMPILER_PREFIX := xtensa-$(ESP_VARIANT)-elf
else
ESP32_COMPILER_PREFIX := riscv32-esp-elf
IDF_TARGET_ARCH_RISCV := 1
# This is important as no hardware FPU is available on these SOCs
ifeq ($(IDF_VERSION),v5.2)
ESP32_RISCV_ARCH := rv32imc_zicsr_zifencei
else
ESP32_RISCV_ARCH := rv32imc
endif
endif

# $1 => Tool sub-path/name
define FindTool
$(lastword $(sort $(wildcard $(IDF_TOOLS_PATH)/$1*)))
endef

DEBUG_VARS			+= ESP32_COMPILER_PATH ESP32_ULP_PATH ESP32_OPENOCD_PATH ESP32_PYTHON_PATH

ifndef ESP32_ULP_PATH
ESP32_ULP_PATH		:= $(call FindTool,tools/$(ESP_VARIANT)ulp-elf/)
endif

ifndef ESP32_OPENOCD_PATH
ESP32_OPENOCD_PATH	:= $(call FindTool,tools/openocd-esp32/)
endif

ifndef ESP32_PYTHON_PATH
ESP32_PYTHON_PATH := $(call FindTool,python_env/idf$(subst v,,$(IDF_VERSION)))
ifndef ESP32_PYTHON_PATH
ESP32_PYTHON_PATH := $(dir $(PYTHON))
else ifeq ($(UNAME),Windows)
ESP32_PYTHON := $(ESP32_PYTHON_PATH)/Scripts/python
else
ESP32_PYTHON := $(ESP32_PYTHON_PATH)/bin/python
endif
endif

# Required by v4.2 SDK
export IDF_PYTHON_ENV_PATH=$(ESP32_PYTHON_PATH)

# Add ESP-IDF tools to PATH
IDF_PATH_LIST := \
	$(IDF_PATH)/tools \
	$(ESP32_COMPILER_PATH) \
	$(ESP32_ULP_PATH)/$(ESP_VARIANT)ulp-elf-binutils/bin \
	$(ESP32_OPENOCD_PATH)/openocd-esp32/bin

ifeq ($(UNAME),Windows)
DEBUG_VARS += ESP32_NINJA_PATH
ifndef ESP32_NINJA_PATH
ESP32_NINJA_PATH	:= $(call FindTool,tools/ninja/)
endif
ifeq (,$(wildcard $(ESP32_NINJA_PATH)/ninja.exe))
$(error Failed to find NINJA)
endif
IDF_PATH_LIST += $(ESP32_NINJA_PATH)

DEBUG_VARS += ESP32_IDFEXE_PATH
ifndef ESP32_IDFEXE_PATH
ESP32_IDFEXE_PATH := $(call FindTool,tools/idf-exe/)
endif
IDF_PATH_LIST += $(ESP32_IDFEXE_PATH)
endif

ifeq ($(ENABLE_CCACHE),1)
export IDF_CCACHE_ENABLE := 1
endif

DEBUG_VARS += NINJA
NINJA := $(if $(ESP32_NINJA_PATH),$(ESP32_NINJA_PATH)/,)ninja

empty:=
space:= $(empty) $(empty)

export PATH := $(subst $(space),:,$(IDF_PATH_LIST)):$(PATH)

TOOLSPEC 	:= $(ESP32_COMPILER_PATH)/$(ESP32_COMPILER_PREFIX)
AS			:= $(TOOLSPEC)-gcc
CC			:= $(TOOLSPEC)-gcc
CXX			:= $(TOOLSPEC)-g++
AR			:= $(TOOLSPEC)-ar
LD			:= $(TOOLSPEC)-gcc
NM			:= $(TOOLSPEC)-nm
OBJCOPY		:= $(TOOLSPEC)-objcopy
OBJDUMP		:= $(TOOLSPEC)-objdump
SIZE 		:= $(TOOLSPEC)-size

ifeq (None,$(ESP32_GDB_PATH))
GDB			:= $(TOOLSPEC)-gdb
else
GDB			:= $(IDF_TOOLS_PATH)/tools/$(ESP32_GDB_PATH)/$(ESP32_COMPILER_PREFIX)-gdb
endif


# [ Sming specific flags ]
DEBUG_VARS += IDF_PATH IDF_VER

# Common C/C++ flags
CPPFLAGS += \
	-DESP_PLATFORM \
	-DIDF_VER=\"$(IDF_VER)\" \
	-MMD \
	-MP \
	$(EXTRA_CPPFLAGS) \
	-DARCH_ESP32 \
	-DESP32 \
	-D__ESP32_EX__ \
	-D__ets__ \
	-D_GNU_SOURCE \
	-DCONFIG_NONE_OS \
	-Dasm=__asm__ \
	-Dtypeof=__typeof__

PROJECT_VER ?=
export IDF_VER
export PROJECT_NAME
export PROJECT_VER

# Flags which control code generation and dependency generation, both for C and C++
CPPFLAGS += -Wno-frame-address

ifdef IDF_TARGET_ARCH_RISCV
CPPFLAGS += \
	-march=$(ESP32_RISCV_ARCH)
else
CPPFLAGS += \
	-mlongcalls \
	-mtext-section-literals
endif

ifeq ($(SMING_RELEASE),1)
CPPFLAGS += -freorder-blocks
endif

# => Tools
MEMANALYZER = $(PYTHON) $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
