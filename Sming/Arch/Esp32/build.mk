##############
#
# Esp32 Architecture build environment
#
##############

ifeq (,$(IDF_PATH))
$(error IDF_PATH not set correctly: "$(IDF_PATH)")
endif

export IDF_PATH := $(call FixPath,$(IDF_PATH))

# By default, downloaded tools will be installed under $HOME/.espressif directory
# (%USERPROFILE%/.espressif on Windows). This path can be modified by setting
# IDF_TOOLS_PATH variable prior to running this tool.
DEBUG_VARS += IDF_TOOLS_PATH
ifeq ($(UNAME),Windows)
IDF_TOOLS_PATH ?= $(USERPROFILE)/.espressif
else
IDF_TOOLS_PATH ?= $(HOME)/.espressif
endif

export IDF_TOOLS_PATH := $(call FixPath,$(IDF_TOOLS_PATH))

ifndef ESP_VARIANT
override ESP_VARIANT := esp32
endif

export ESP_VARIANT

ifeq ($(ESP_VARIANT),esp32c3)
ESP32_COMPILER_PREFIX := riscv32-esp-elf
IDF_TARGET_ARCH_RISCV := 1
else
ESP32_COMPILER_PREFIX := xtensa-$(ESP_VARIANT)-elf
endif

# $1 => Root directory
# $2 => Sub-directory
define FindTool
$(lastword $(sort $(wildcard $(IDF_TOOLS_PATH)/$1/*))$2)
endef

DEBUG_VARS			+= ESP32_COMPILER_PATH ESP32_ULP_PATH ESP32_OPENOCD_PATH ESP32_PYTHON_PATH

ifndef ESP32_COMPILER_PATH
include $(IDF_PATH)/tools/toolchain_versions.mk
ESP32_COMPILER_PATH	:= $(IDF_TOOLS_PATH)/tools/$(ESP32_COMPILER_PREFIX)/$(CURRENT_TOOLCHAIN_COMMIT_DESC)-$(CURRENT_TOOLCHAIN_GCC_VERSION)/$(ESP32_COMPILER_PREFIX)
endif

ifndef ESP32_ULP_PATH
ESP32_ULP_PATH		:= $(call FindTool,tools/$(ESP_VARIANT)ulp-elf)
endif

ifndef ESP32_OPENOCD_PATH
ESP32_OPENOCD_PATH	:= $(call FindTool,tools/openocd-esp32)
endif

ifndef ESP32_PYTHON_PATH
ESP32_PYTHON_PATH	:= $(call FindTool,python_env)
ifneq (,$(wildcard $(ESP32_PYTHON_PATH)/bin))
ESP32_PYTHON_PATH := $(ESP32_PYTHON_PATH)/bin
else ifneq (,$(wildcard $(ESP32_PYTHON_PATH)/Scripts))
ESP32_PYTHON_PATH := $(ESP32_PYTHON_PATH)/Scripts
else
$(error Failed to find ESP32 Python installation)
endif
ESP32_PYTHON = $(ESP32_PYTHON_PATH)/python
endif

# Required by v4.2 SDK
export IDF_PYTHON_ENV_PATH=$(ESP32_PYTHON_PATH)

# Add ESP-IDF tools to PATH
IDF_PATH_LIST := \
	$(IDF_PATH)/tools \
	$(ESP32_COMPILER_PATH)/bin \
	$(ESP32_ULP_PATH)/$(ESP_VARIANT)ulp-elf-binutils/bin \
	$(ESP32_OPENOCD_PATH)/openocd-esp32/bin \
	$(ESP32_PYTHON_PATH)/bin \
	$(IDF_PATH)/components/esptool_py/esptool \
	$(IDF_PATH)/components/espcoredump \
	$(IDF_PATH)/components/partition_table

ifeq ($(UNAME),Windows)
DEBUG_VARS += ESP32_NINJA_PATH
ifndef ESP32_NINJA_PATH
ESP32_NINJA_PATH	:= $(call FindTool,tools/ninja)
endif
ifeq (,$(wildcard $(ESP32_NINJA_PATH)/ninja.exe))
$(error Failed to find NINJA)
endif
IDF_PATH_LIST += $(ESP32_NINJA_PATH)

DEBUG_VARS += ESP32_IDFEXE_PATH
ifndef ESP32_IDFEXE_PATH
ESP32_IDFEXE_PATH := $(call FindTool,tools/idf-exe)
endif
IDF_PATH_LIST += ESP32_IDFEXE_PATH
endif

DEBUG_VARS += NINJA
NINJA := $(if $(ESP32_NINJA_PATH),$(ESP32_NINJA_PATH)/,)ninja

space :=
space +=

export PATH := $(subst $(space),:,$(IDF_PATH_LIST)):$(PATH)

TOOLSPEC 	:= $(ESP32_COMPILER_PATH)/bin/$(ESP32_COMPILER_PREFIX)
AS			:= $(TOOLSPEC)-gcc
CC			:= $(TOOLSPEC)-gcc
CXX			:= $(TOOLSPEC)-g++
AR			:= $(TOOLSPEC)-ar
LD			:= $(TOOLSPEC)-gcc
NM			:= $(TOOLSPEC)-nm
OBJCOPY		:= $(TOOLSPEC)-objcopy
OBJDUMP		:= $(TOOLSPEC)-objdump
GDB			:= $(TOOLSPEC)-gdb
SIZE 		:= $(TOOLSPEC)-size

# Get version variables
include $(IDF_PATH)/make/version.mk
include $(IDF_PATH)/make/ldgen.mk

# If we have `version.txt` then prefer that for extracting IDF version
ifeq ("$(wildcard ${IDF_PATH}/version.txt)","")
IDF_VER_T := $(shell cd ${IDF_PATH} && git describe --always --tags --dirty)
else
IDF_VER_T := $(shell cat ${IDF_PATH}/version.txt)
endif
IDF_VER := $(shell echo "$(IDF_VER_T)"  | cut -c 1-31)

# [ Sming specific flags ]
DEBUG_VARS += IDF_PATH IDF_VER

# IDF uses 'asm' keyword
SMING_C_STD := gnu11

# Common C/C++ flags
CPPFLAGS += \
	-DESP_PLATFORM \
	-DIDF_VER=\"$(IDF_VER)\" \
	-MMD \
	-MP \
	$(EXTRA_CPPFLAGS) \
	-DARCH_ESP32 \
	-D__ets__ \
	-D_GNU_SOURCE \
	-DCONFIG_NONE_OS

PROJECT_VER ?=
export IDF_VER
export PROJECT_NAME
export PROJECT_VER

# Flags which control code generation and dependency generation, both for C and C++
CPPFLAGS += -Wno-frame-address

ifndef IDF_TARGET_ARCH_RISCV
CPPFLAGS += \
	-mlongcalls \
	-mtext-section-literals
endif

ifeq ($(SMING_RELEASE),1)
CPPFLAGS += -freorder-blocks
endif

# => Tools
MEMANALYZER = $(PYTHON) $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
