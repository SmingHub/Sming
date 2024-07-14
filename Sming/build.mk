# Build environment definitions

include $(SMING_HOME)/util.mk

# Cached build variables
BUILD_VARS :=

# Add debug variable names to DEBUG_VARS so they can be easily inspected via `make list-config`
DEBUG_VARS := SMING_HOME
SMING_HOME := $(patsubst %/,%,$(call FixPath,$(SMING_HOME)))

# Resolve SMING_ARCH and SMING_SOC settings
DEBUG_VARS += SMING_ARCH SMING_SOC
ifeq (,$(SMING_ARCH))
  ifeq (,$(SMING_SOC))
    override SMING_ARCH := Esp8266
    override SMING_SOC := esp8266
  else
    override SMING_ARCH := $(notdir $(call dirx,$(filter %/$(SMING_SOC)-soc.json,$(SOC_CONFIG_FILES))))
    ifeq (,$(SMING_ARCH))
      $(error SOC '$(SMING_SOC)' not found)
    endif
  endif
else ifeq (,$(filter $(SMING_SOC),$(ARCH_$(SMING_ARCH)_SOC)))
  override SMING_SOC := $(firstword $(ARCH_$(SMING_ARCH)_SOC))
endif

ifeq (,$(wildcard $(SMING_HOME)/Arch/$(SMING_ARCH)/build.mk))
  $(error Arch '$(SMING_ARCH)' not found)
endif

BUILD_VARS += CLANG_TIDY
ifdef CLANG_TIDY
ifneq (Host,$(SMING_ARCH))
  $(error CLANG_TIDY supported only for Host architecture.)
endif
USE_CLANG := 1
endif

export SMING_ARCH
export SMING_SOC

# SOC config for currently selected variant
export SOC_CONFIG_FILE := $(SMING_HOME)/Arch/$(SMING_ARCH)/$(SMING_SOC)-soc.json

# Paths for standard build tools
DEBUG_VARS += \
	AS \
	CC \
	CXX \
	AR \
	LD \
	NINJA \
	NM \
	OBJCOPY \
	OBJDUMP \
	GDB

ifdef NINJA
NINJA := $(call FixPath,$(NINJA))
else
NINJA := ninja
endif

DEBUG_VARS		+= SMING_RELEASE
ifeq ($(SMING_RELEASE),1)
	BUILD_TYPE	:= release
else
	BUILD_TYPE	:= debug
	# Simplifies logic if =0
	override SMING_RELEASE :=
endif
export SMING_RELEASE

SMING_TOOLS := $(realpath $(SMING_HOME)/../Tools)

# Detect OS and build environment
TOOL_EXT	:=
DEBUG_VARS	+= UNAME OS
UNAME		:= $(shell uname -s)
ifneq ($(filter MINGW32_NT%,$(UNAME)),)
	UNAME		:= Windows
	TOOL_EXT	:= .exe
	# May be required by some applications (e.g. openssl)
	HOME ?= $(USERPROFILE)
	export HOME
else ifneq ($(filter CYGWIN%,$(UNAME)),)
	# Cygwin Detected
	UNAME		:= Linux
else ifneq ($(filter CYGWIN%WOW,$(UNAME)),)
	#Cygwin32
	UNAME		:= Linux
else ifneq ($(filter MSYS%WOW,$(UNAME)),)
	#Msys32
	UNAME		:= Linux
else ifeq ($(UNAME), Linux)
 	#Linux
 	# Detect WSL (Windows Subsystem for Linux)
 	ifdef WSL_DISTRO_NAME
        DEBUG_VARS += WSL_ROOT
        WSL_ROOT := //wsl$$/$(WSL_DISTRO_NAME)
		# If serial device is available, use it directly, otherwise via powershell
		WSL_COMPORT_POWERSHELL = $(if $(wildcard $(COM_PORT)),,1)
 	endif
else ifeq ($(UNAME), Darwin)
 	#OS X
else ifeq ($(UNAME), Freebsd)
 	#BSD
endif

MAKECMDGOALS	?= all

# Some components play nicer if they avoid doing stuff doing a cleanup
ifneq (,$(findstring clean,$(MAKECMDGOALS)))
export MAKE_CLEAN := 1
endif

export SMING_HOME
export COMPILE := gcc

DEBUG_VARS		+= ARCH_BASE

ARCH_BASE		:= $(SMING_HOME)/Arch/$(SMING_ARCH)
ARCH_CORE		= $(ARCH_BASE)/Core
ARCH_TOOLS		= $(ARCH_BASE)/Tools
ARCH_COMPONENTS	= $(ARCH_BASE)/Components


# Git command
DEBUG_VARS	+= GIT
GIT ?= git

# ccache
DEBUG_VARS += CCACHE
CCACHE ?= ccache

# CMake command
DEBUG_VARS	+= CMAKE
CMAKE ?= cmake

# clang-format command
DEBUG_VARS	+= CLANG_FORMAT
CLANG_FORMAT ?= clang-format

# more tools
DEBUG_VARS += AWK
# In case 'awk' is an alias for 'gawk' on your system, having 'POSIXLY_CORRECT' in the environment
# invokes an awk compatibility mode. It has no effect on other awk implementations.
AWK ?= POSIXLY_CORRECT= awk

DEBUG_VARS += SED
ifeq ($(UNAME),Darwin)
SED ?= gsed
else
SED ?= sed
endif

# Python command
DEBUG_VARS += PYTHON
ifdef PYTHON
export PYTHON := $(call FixPath,$(PYTHON))
else
PYTHON := $(shell which python)
endif

PYTHON_VERSION := $(shell $(PYTHON) --version 2>&1)
$(if $V,$(info PYTHON_VERSION = $(PYTHON_VERSION)))
PYTHON_VERSION_OK := $(findstring Python,$(PYTHON_VERSION))
ifndef PYTHON_VERSION_OK
$(error Cannot find Python installation - check PATH or PYTHON environment variables)
endif


# Common C/C++ flags passed to user libraries
CPPFLAGS = \
	-finline-functions \
	-fdata-sections \
	-ffunction-sections \
	-D_POSIX_C_SOURCE=200809L \
	-Wall \
	-Wpointer-arith \
	-Wno-comment \
	-DARDUINO=106

# If STRICT is enabled, show all warnings but don't treat as errors
BUILD_VARS += STRICT
STRICT ?= 0
export STRICT
ifeq ($(STRICT),1)
CPPFLAGS += \
	-Wimplicit-fallthrough \
	-Wunused-parameter \
	-Wunused-but-set-parameter
else
CPPFLAGS += \
	-Werror \
	-Wno-sign-compare \
	-Wno-parentheses \
	-Wno-unused-variable \
	-Wno-unused-but-set-variable \
	-Wno-strict-aliasing
endif

ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	# Note: ANSI requires NDEBUG to be defined for correct assert behaviour
	CPPFLAGS	+= -Os -DSMING_RELEASE=1 -DNDEBUG
else ifeq ($(ENABLE_GDB), 1)
	ifeq ($(SMING_ARCH),Host)
		CPPFLAGS	+= -O0
	else
		CPPFLAGS	+= -Og
	endif
else
	CPPFLAGS	+= -Os -g
endif

CXXFLAGS += \
	-felide-constructors \
	-fno-rtti \
	-fno-exceptions

ifneq ($(STRICT),1)
	CXXFLAGS += -Wno-reorder
endif

# ccache can speed up re-builds considerably
BUILD_VARS += ENABLE_CCACHE
ENABLE_CCACHE ?= 0

include $(ARCH_BASE)/build.mk

ifndef MAKE_CLEAN

# Detect compiler version and name
DEBUG_VARS				+= COMPILER_VERSION_FULL COMPILER_VERSION COMPILER_NAME
COMPILER_VERSION_FULL	:= $(shell LANG=C $(CC) -v 2>&1 | $(AWK) -F " version " '/ version /{ a=$$1; gsub(/ +/, "-", a); print a, $$2}')
COMPILER_NAME			:= $(word 1,$(COMPILER_VERSION_FULL))
COMPILER_VERSION		:= $(word 2,$(COMPILER_VERSION_FULL))

ifndef USE_CLANG
# Required to access peripheral registers using structs
# e.g. `uint32_t value: 8` sitting at a byte or word boundary will be 'optimised' to
# an 8-bit fetch/store instruction which will not work; it must be a full 32-bit access.
ifeq ($(COMPILER_NAME),gcc)
CPPFLAGS += -fstrict-volatile-bitfields
COMPILER_VERSION_MIN := 8
else
ifeq (,$(findstring clang,$(COMPILER_NAME)))
$(shell LANG=C $(CC) -v)
$(error Compiler '$(COMPILER_VERSION_FULL)' not recognised. Please install GCC tools.)
endif
ifndef COMPILER_NOTICE_PRINTED
$(info Note: Building with $(COMPILER_NAME) $(COMPILER_VERSION).)
COMPILER_NOTICE_PRINTED := 1
endif
USE_CLANG := 1
endif
endif

ifdef USE_CLANG
COMPILER_VERSION_MIN := 14
CPPFLAGS += \
	-Wno-vla-extension \
	-Wno-unused-private-field \
	-Wno-bitfield-constant-conversion \
	-Wno-unknown-pragmas \
	-Wno-initializer-overrides
endif

# Sanitizers
BUILD_VARS += ENABLE_SANITIZERS SANITIZERS
ENABLE_SANITIZERS ?= 0
SANITIZERS ?= \
	address \
	pointer-compare \
	pointer-subtract \
	leak \
	undefined
ifeq ($(ENABLE_SANITIZERS),1)
CPPFLAGS += \
	-fstack-protector-all \
	-fsanitize-address-use-after-scope \
	$(foreach s,$(SANITIZERS),-fsanitize=$s)
endif

# Use c11 by default. Every architecture can override it
DEBUG_VARS			+= SMING_C_STD
SMING_C_STD			?= c11
CFLAGS				+= -std=$(SMING_C_STD)

# Select C++17 if supported, defaulting to C++11 otherwise
DEBUG_VARS			+= SMING_CXX_STD
SMING_CXX_STD		?= c++17
CXXFLAGS			+= -std=$(SMING_CXX_STD)

COMPILER_VERSION_MAJOR := $(firstword $(subst ., ,$(COMPILER_VERSION)))
COMPILER_VERSION_COMPATIBLE := $(shell expr $(COMPILER_VERSION_MAJOR) \>= $(COMPILER_VERSION_MIN))

ifeq ($(COMPILER_VERSION_COMPATIBLE),0)
ifneq ($(GCC_UPGRADE_URL),)
$(info Instructions for upgrading your compiler can be found here: $(GCC_UPGRADE_URL))
endif
$(error Please upgrade your compiler to $(COMPILER_NAME) $(COMPILER_VERSION_MIN) or newer)
endif
endif

DEBUG_VARS		+= USER_LIBDIR OUT_BASE BUILD_BASE FW_BASE TOOLS_BASE SMING_ARCH_FULL

# Architectures with multiple variants require an extra subdirectory
ifeq (,$(word 2,$(ARCH_$(SMING_ARCH)_SOC)))
SMING_ARCH_FULL	:= $(SMING_ARCH)
else
SMING_ARCH_FULL := $(SMING_ARCH)/$(SMING_SOC)
endif

OUT_BASE		:= out/$(SMING_ARCH_FULL)/$(BUILD_TYPE)
BUILD_BASE		= $(OUT_BASE)/build
FW_BASE			= $(OUT_BASE)/firmware
TOOLS_BASE		= $(SMING_HOME)/$(OUT_BASE)/tools
USER_LIBDIR		= $(SMING_HOME)/$(OUT_BASE)/lib

# Component (user) libraries have a special prefix so linker script can identify them
CLIB_PREFIX := clib-

# Use with LDFLAGS to define a symbol alias
# $1 -> List of alias=name pairs
ifeq ($(UNAME)$(SMING_ARCH),DarwinHost)
DefSym = $(foreach n,$1,-Wl,-alias,_$(word 2,$(subst =, ,$n)),_$(word 1,$(subst =, ,$n)))
else
DefSym = $(foreach n,$1,-Wl,--defsym=$n)
endif

# Use with LDFLAGS to undefine a list of symbols
# $1 -> List of symbols
define Undef
$(foreach n,$1,-u $n)
endef

# Use with LDFLAGS to wrap a list of symbols
# $1 -> List of symbols
define Wrap
$(foreach n,$1,-Wl,-wrap,$n)
endef

# Use with LDFLAGS to undefine and wrap a list of symbols
# $1 -> List of symbols
define UndefWrap
$(foreach n,$1,-u $n -Wl,-wrap,$n)
endef

# Apply coding style to list of files using clang-format
# $1 -> List of files
define ClangFormat
	$(if $(V),$(info Applying coding style to $(words $1) files ...))
	$(call ClangFormatBatch,$1)
endef

define ClangFormatBatch
	@$(CLANG_FORMAT) -i -style=file $(wordlist 1,20,$1)
	$(if $(word 21,$1),$(call ClangFormatBatch,$(wordlist 21,1000000,$1)))
endef

define ListSubmodules
$(call ScanGitModules,$(SMING_HOME)/..,.*)
endef

# Extract commented target information from makefiles and display
define PrintHelp
	@echo
	@echo Welcome to the Sming build system!
	@$(AWK) -f $(SMING_HOME)/help.awk $(foreach f,$(MAKEFILE_LIST),"$(f)")
	@echo
endef


# Write config variables to a file
# $1 -> Output filename
# $2 -> List of variable names to output
define WriteConfig
$(if $(V),$(info WriteConfig $1))
$(shell	mkdir -p $(dir $1);
	echo '# Automatically generated file. Do not edit.' > $1;
	echo >> $1;
	$(foreach v,$2,echo '$v = $($v)' >> $1;) )
endef


## PATCHING

# Apply patch to a submodule
# $1 -> patch file with relative path
define ApplyPatch
	$(GIT) apply -v $1 --ignore-whitespace --whitespace=nowarn
endef

# If there's a patch for this submodule, apply it
# We look for patch in .. and in ../.patches
# If a matching subdirectory is found in ../.patches/ then the contents
# are copied, overwriting any existing files with the same name
# $1 -> submodule path
# $2 -> name of patch file
define TryApplyPatch
	cd $1 && if [ -f ../$2 ]; then \
		$(call ApplyPatch,../$2); \
	elif [ -f ../.patches/$2 ]; then \
		$(call ApplyPatch,../.patches/$2); \
	fi && \
	if [ -d ../.patches/$(basename $2)/ ]; then \
		cp -rf ../.patches/$(basename $2)/* . ; \
	fi
endef

# Fetch and patch a submodule
# $1 -> submodule parent path
# $2 -> submodule name
define FetchAndPatch
	$(info )
	$(info Fetching submodule '$2' ...)
	$(Q)	cd $1 && ( \
				rm -rf $2; \
				if [ -f "$2.no-recursive" ]; then OPTS=""; else OPTS="--recursive"; fi; \
				$(GIT) submodule update --init --force $$OPTS $2 \
			)
	$(Q) $(call TryApplyPatch,$1/$2,$2.patch)
endef

# Update and patch submodule
# Patch file is either in submodule parent directory itself or subdirectory .patches from there
%/.submodule:
	$(call FetchAndPatch,$(abspath $*/..),$(*F))
	$(Q) touch $@
