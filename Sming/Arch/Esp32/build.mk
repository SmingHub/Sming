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
ESP_VARIANT := esp32
endif

ESP32_COMPILER_PREFIX := xtensa-$(ESP_VARIANT)-elf

# $1 => Root directory
# $2 => Sub-directory
define FindTool
$(lastword $(sort $(wildcard $(IDF_TOOLS_PATH)/$1/*))$2)
endef

DEBUG_VARS			+= ESP32_COMPILER_PATH ESP32_ULP_PATH ESP32_OPENOCD_PATH ESP32_PYTHON_PATH
ifndef ESP32_COMPILER_PATH
ESP32_COMPILER_PATH	:= $(call FindTool,tools/$(ESP32_COMPILER_PREFIX),/$(ESP32_COMPILER_PREFIX))
endif
ifndef ESP32_ULP_PATH
ESP32_ULP_PATH		:= $(call FindTool,tools/$(ESP_VARIANT)ulp-elf)
endif
ifndef ESP32_OPENOCD_PATH
ESP32_OPENOCD_PATH	:= $(call FindTool,tools/openocd-esp32)
endif
ifndef ESP32_PYTHON_PATH
ESP32_PYTHON_PATH	:= $(call FindTool,python_env)
endif

# Required by v4.2 SDK
export IDF_PYTHON_ENV_PATH=$(ESP32_PYTHON_PATH)

# Add ESP-IDF tools to PATH
IDF_PATH_LIST := \
	$(ESP32_COMPILER_PATH)/bin \
	$(ESP32_ULP_PATH)/$(ESP_VARIANT)ulp-elf-binutils/bin \
	$(ESP32_OPENOCD_PATH)/openocd-esp32/bin \
	$(ESP32_PYTHON_PATH)/bin \
	$(IDF_PATH)/components/esptool_py/esptool \
	$(IDF_PATH)/components/espcoredump \
	$(IDF_PATH)/components/partition_table

space :=
space +=

export PATH := $(subst $(space),:,$(IDF_PATH_LIST)):$(PATH)

TOOLSPEC 	:= $(ESP32_COMPILER_PATH)/bin/$(ESP32_COMPILER_PREFIX)
AS			:= $(TOOLSPEC)-gcc
CC			:= $(TOOLSPEC)-gcc
CXX			:= $(TOOLSPEC)-g++
AR			:= $(TOOLSPEC)-ar
LD			:= $(TOOLSPEC)-gcc
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
DEBUG_VARS			+= IDF_PATH IDF_VER

# Set default LDFLAGS
EXTRA_LDFLAGS ?=
LDFLAGS ?= -nostdlib \
	-u call_user_start_cpu0	\
	$(EXTRA_LDFLAGS) \
	-Wl,--gc-sections	\
	-Wl,-static	\
	-Wl,--start-group	\
	$(COMPONENT_LDFLAGS) \
	-lgcc \
	-lstdc++ \
	-lgcov \
	-Wl,--end-group \
	-Wl,-EL
	
SMING_C_STD := gnu99

# Set default CPPFLAGS, CFLAGS, CXXFLAGS
# These are exported so that components can use them when compiling.
# If you need your component to add CFLAGS/etc for it's own source compilation only, set CFLAGS += in your component's Makefile.
# If you need your component to add CFLAGS/etc globally for all source
#  files, set CFLAGS += in your component's Makefile.projbuild
# If you need to set CFLAGS/CPPFLAGS/CXXFLAGS at project level, set them in application Makefile
#  before including project.mk. Default flags will be added before the ones provided in application Makefile.

# This variable stores the common C/C++ flags
# CPPFLAGS used by C preprocessor
# If any flags are defined in application Makefile, add them at the end.
CPPFLAGS += -DESP_PLATFORM -D IDF_VER=\"$(IDF_VER)\" -MMD -MP $(EXTRA_CPPFLAGS)

# Sming specific CPPFLAGS
CPPFLAGS += \
	-DARCH_ESP32 \
	-D__ets__ \
	-DICACHE_FLASH \
	-DUSE_OPTIMIZE_PRINTF \
	-DESP32

PROJECT_VER ?=
export IDF_VER
export PROJECT_NAME
export PROJECT_VER

# Warnings-related flags relevant both for C and C++
COMMON_WARNING_FLAGS := -Wall -Werror=all \
	-Wno-error=unused-function \
	-Wno-error=unused-but-set-variable \
	-Wno-error=unused-variable \
	-Wno-error=deprecated-declarations \
	-Wno-error=extra \
	-Wno-unused-parameter -Wno-error=sign-compare \
	-Wno-error=ignored-qualifiers \
	-Wno-error=missing-field-initializers \
	-Wno-error=implicit-fallthrough
	
# Sming warning....
COMMON_WARNING_FLAGS += -Wno-error=undef

ifdef CONFIG_COMPILER_DISABLE_GCC8_WARNINGS
COMMON_WARNING_FLAGS += -Wno-parentheses \
	-Wno-sizeof-pointer-memaccess \
	-Wno-clobbered \
	-Wno-format-overflow \
	-Wno-stringop-truncation \
	-Wno-misleading-indentation \
	-Wno-cast-function-type \
	-Wno-implicit-fallthrough \
	-Wno-unused-const-variable \
	-Wno-switch-unreachable \
	-Wno-format-truncation \
	-Wno-memset-elt-size \
	-Wno-int-in-bool-context
endif

ifdef CONFIG_COMPILER_WARN_WRITE_STRINGS
COMMON_WARNING_FLAGS += -Wwrite-strings
endif #CONFIG_COMPILER_WARN_WRITE_STRINGS

# Flags which control code generation and dependency generation, both for C and C++
COMMON_FLAGS := \
	-Wno-frame-address \
	-ffunction-sections -fdata-sections \
	-fstrict-volatile-bitfields \
	-mlongcalls \
	-mtext-section-literals \
	-nostdlib

ifndef IS_BOOTLOADER_BUILD
# stack protection (only one option can be selected in menuconfig)
ifdef CONFIG_COMPILER_STACK_CHECK_MODE_NORM
COMMON_FLAGS += -fstack-protector
endif
ifdef CONFIG_COMPILER_STACK_CHECK_MODE_STRONG
COMMON_FLAGS += -fstack-protector-strong
endif
ifdef CONFIG_COMPILER_STACK_CHECK_MODE_ALL
COMMON_FLAGS += -fstack-protector-all
endif
endif

# Optimization flags are set based on menuconfig choice
ifdef CONFIG_COMPILER_OPTIMIZATION_LEVEL_RELEASE
OPTIMIZATION_FLAGS = -Os -freorder-blocks
else
OPTIMIZATION_FLAGS = -Og
endif

ifdef CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_DISABLE
CPPFLAGS += -DNDEBUG
endif

# IDF uses some GNU extension from libc
CPPFLAGS += -D_GNU_SOURCE

CPPFLAGS += -DCONFIG_NONE_OS

# Enable generation of debugging symbols
# (we generate even in Release mode, as this has no impact on final binary size.)
DEBUG_FLAGS ?= -ggdb

# List of flags to pass to C compiler
# If any flags are defined in application Makefile, add them at the end.
CFLAGS := $(strip \
	$(OPTIMIZATION_FLAGS) $(DEBUG_FLAGS) \
	$(COMMON_FLAGS) \
	$(COMMON_WARNING_FLAGS) -Wno-old-style-declaration \
	$(CPPFLAGS) \
	$(CFLAGS) \
	$(EXTRA_CFLAGS))

# List of flags to pass to C++ compiler
# If any flags are defined in application Makefile, add them at the end.
CXXFLAGS := $(strip \
	-std=gnu++11 \
	$(OPTIMIZATION_FLAGS) $(DEBUG_FLAGS) \
	$(COMMON_FLAGS) \
	$(COMMON_WARNING_FLAGS) \
	$(CPPFLAGS) \
	$(CXXFLAGS) \
	$(EXTRA_CXXFLAGS))

ifdef CONFIG_COMPILER_CXX_EXCEPTIONS
CXXFLAGS += -fexceptions
else
CXXFLAGS += -fno-exceptions
endif

ifdef CONFIG_COMPILER_CXX_RTTI
CXXFLAGS += -frtti
else
CXXFLAGS += -fno-rtti
LDFLAGS += -fno-rtti
endif

ARFLAGS := cru

# => Tools
MEMANALYZER = $(PYTHON) $(ARCH_TOOLS)/memanalyzer.py $(OBJDUMP)$(TOOL_EXT)
