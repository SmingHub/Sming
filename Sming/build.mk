# Build environment definitions

include $(SMING_HOME)/util.mk

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

ifdef CLANG_TIDY
ifneq (Host,$(SMING_ARCH))
  $(error CLANG_TIDY supported only for Host architecture.)
endif
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

# Python command
DEBUG_VARS += PYTHON
ifdef PYTHON
export PYTHON := $(call FixPath,$(PYTHON))
else
PYTHON := python3
endif

PYTHON_VERSION := $(shell $(PYTHON) --version 2>&1)
$(if $V,$(info PYTHON_VERSION = $(PYTHON_VERSION)))
PYTHON_VERSION_OK := $(findstring Python,$(PYTHON_VERSION))
ifndef PYTHON_VERSION_OK
$(error Cannot find Python installation - check PATH or PYTHON environment variables)
endif


# Common C/C++ flags passed to user libraries
CPPFLAGS = \
	-Wl,-EL \
	-finline-functions \
	-fdata-sections \
	-ffunction-sections

# Required to access peripheral registers using structs
# e.g. `uint32_t value: 8` sitting at a byte or word boundary will be 'optimised' to
# an 8-bit fetch/store instruction which will not work; it must be a full 32-bit access.
CPPFLAGS += -fstrict-volatile-bitfields

CPPFLAGS += \
	-Wall \
	-Wpointer-arith \
	-Wno-comment \
	-DARDUINO=106

# If STRICT is enabled, show all warnings but don't treat as errors
DEBUG_VARS += STRICT
STRICT ?= 0
export STRICT
ifneq ($(STRICT),1)
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

include $(ARCH_BASE)/build.mk

ifndef MAKE_CLEAN

# Detect compiler version
DEBUG_VARS			+= GCC_VERSION
GCC_VERSION			:= $(shell $(CC) -dumpversion)

# Use c11 by default. Every architecture can override it
DEBUG_VARS			+= SMING_C_STD
SMING_C_STD			?= c11
CFLAGS				+= -std=$(SMING_C_STD)

# Select C++17 if supported, defaulting to C++11 otherwise
DEBUG_VARS			+= SMING_CXX_STD
ifeq ($(GCC_VERSION),4.8.5)
SMING_CXX_STD		?= c++11
else
SMING_CXX_STD		?= c++17
endif
CXXFLAGS			+= -std=$(SMING_CXX_STD)

GCC_MIN_MAJOR_VERSION := 8
GCC_VERSION_COMPATIBLE := $(shell expr $$(echo $(GCC_VERSION) | cut -f1 -d.) \>= $(GCC_MIN_MAJOR_VERSION))

ifeq ($(GCC_VERSION_COMPATIBLE),0)
$(warning ***** Please, upgrade your GCC compiler to version $(GCC_MIN_MAJOR_VERSION) or newer *****)
ifneq ($(GCC_UPGRADE_URL),)
$(info Instructions for upgrading your compiler can be found here: $(GCC_UPGRADE_URL))
endif 
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
define DefSym
$(foreach n,$1,-Wl,--defsym=$n)
endef

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
