##############
#
# Esp32 Architecture build environment
#
##############

ifeq (,$(IDF_PATH))
$(error IDF_PATH not set correctly: "$(IDF_PATH)")
endif

CC 	:= xtensa-esp32-elf-gcc
CXX 	:= xtensa-esp32-elf-g++
LD 	:= xtensa-esp32-elf-gcc
AR 	:= xtensa-esp32-elf-ar
OBJCOPY := xtensa-esp32-elf-objcopy
OBJDUMP := xtensa-esp32-elf-objdump
SIZE 	:= xtensa-esp32-elf-size
AS	:= xtensa-esp32-elf-gcc
GDB	:= xtensa-esp32-elf-gdb

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
DEBUG_VARS			+= IDF_VER

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
