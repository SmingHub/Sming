# Build environment definitions

# Add debug variable names to DEBUG_VARS so they can be easily inspected via `make list-config`
DEBUG_VARS		:=

#
DEBUG_VARS		+= SMING_HOME SMING_ARCH
ifeq (,$(SMING_ARCH))
override SMING_ARCH	:= Esp8266
endif
export SMING_ARCH

DEBUG_VARS		+= SMING_RELEASE
ifeq ($(SMING_RELEASE),1)
	BUILD_TYPE	:= release
else
	BUILD_TYPE	:= debug
	# Simplifies logic if =0
	override SMING_RELEASE :=
endif
export SMING_RELEASE

# Detect OS and build environment
TOOL_EXT	:=
DEBUG_VARS	+= UNAME
UNAME		:= $(shell uname -s)
ifneq ($(filter MINGW32_NT%,$(UNAME)),)
	UNAME		:= Windows
	TOOL_EXT	:= .exe
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
else ifeq ($(UNAME), Darwin)
 	#OS X
else ifeq ($(UNAME), Freebsd)
 	#BSD
endif

# Convert Windows paths to POSIX paths
DEBUG_VARS		+= OS
ifeq ($(OS),Windows_NT)
FixPath			= $(subst //,/,$(subst \,/,$(addprefix /,$(subst :,,$1))))
else
FixPath			= $1
endif

SMING_HOME		:= $(patsubst %/,%,$(call FixPath,$(SMING_HOME)))

ifeq (,$(wildcard $(SMING_HOME)/Arch/$(SMING_ARCH)/build.mk))
$(error Arch '$(SMING_ARCH)' not found)
endif

MAKECMDGOALS	?= all

export SMING_HOME
export COMPILE := gcc

DEBUG_VARS		+= ARCH_BASE USER_LIBDIR OUT_BASE BUILD_BASE FW_BASE TOOLS_BASE

ARCH_BASE		:= $(SMING_HOME)/Arch/$(SMING_ARCH)
ARCH_SYS		= $(ARCH_BASE)/System
ARCH_CORE		= $(ARCH_BASE)/Core
ARCH_TOOLS		= $(ARCH_BASE)/Tools
ARCH_COMPONENTS	= $(ARCH_BASE)/Components

OUT_BASE		:= out/$(SMING_ARCH)/$(BUILD_TYPE)
BUILD_BASE		= $(OUT_BASE)/build
FW_BASE			= $(OUT_BASE)/firmware
TOOLS_BASE		= $(SMING_HOME)/$(OUT_BASE)/tools
USER_LIBDIR		= $(SMING_HOME)/$(OUT_BASE)/lib

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
PYTHON ?= python

V ?= $(VERBOSE)
ifeq ("$(V)","1")
	Q			:=
	vecho		:= @true
else
	Q			:= @
	vecho		:= @echo
endif

# Common flags passed to user libraries
CFLAGS_COMMON = \
	-Wl,-EL \
	-finline-functions \
	-fdata-sections \
	-ffunction-sections

# compiler flags using during compilation of source files. Add '-pg' for debugging
CFLAGS = \
	-Wall \
	-Wundef \
	-Wpointer-arith \
	-Wno-comment \
	$(CFLAGS_COMMON) \
	-DARDUINO=106

# If STRICT is enabled, show all warnings but don't treat as errors
ifneq ($(STRICT),1)
CFLAGS += \
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
	CFLAGS		+= -Os -DSMING_RELEASE=1 -DNDEBUG
else ifeq ($(ENABLE_GDB), 1)
	ifeq ($(SMING_ARCH),Host)
		CFLAGS		+= -O0
	else
		CFLAGS		+= -Og
	endif
else
	CFLAGS		+= -Os -g
endif

CXXFLAGS = $(CFLAGS) -felide-constructors

ifneq ($(STRICT),1)
	CXXFLAGS += -Wno-reorder
endif

include $(ARCH_BASE)/build.mk

# Detect compiler version
DEBUG_VARS			+= GCC_VERSION
GCC_VERSION			:= $(shell $(CC) -dumpversion)

# Select C++17 if supported, defaulting to C++11 otherwise
DEBUG_VARS			+= SMING_CPP_STD
ifeq ($(GCC_VERSION),4.8.5)
SMING_CPP_STD		?= c++11
else
SMING_CPP_STD		?= c++17
endif
CXXFLAGS			+= -std=$(SMING_CPP_STD)

# Component (user) libraries have a special prefix so linker script can identify them
CLIB_PREFIX := clib-

# Apply coding style to list of files using clang-format
# $1 -> List of files
define ClangFormat
	$(if $(V),$(info Applying coding style to $(words $1) files ...))
	@for FILE in $1; do \
		$(CLANG_FORMAT) -i -style=file $$FILE; \
	done
endef

# Calculate a hash string for appending to library names, etc.
# $1 -> Name of variable containing data to be hashed
define CalculateVariantHash
$(firstword $(shell echo -n $($1) | md5sum -t))
endef

# Fetch full path for submodules matching given pattern
# Note that scanning .gitmodules is considerably quicker than using GIT
# $1 -> Path to repo working directory
# $2 -> Path pattern to match
define ScanGitModules
$(patsubst %,$(abspath $1/%),$(subst path = ,,$(shell grep -o 'path = $2' '$1/.gitmodules')))
endef

define ListSubmodules
$(call ScanGitModules,$(SMING_HOME)/..,.*)
endef

# List immediate sub-directories for a list of root directories
# Results are sorted and without trailing path separator
# $1 -> Root paths
define ListSubDirs
$(foreach d,$(dir $(wildcard $1/*/.)),$(d:/=))
endef

# Check that $2 is a valid sub-directory of $1. Return empty string if not.
# $1 -> Parent directory
# $2 -> Sub-directory
# During wildcard searches, paths with spaces cause recursion.
define IsSubDir
$(if $(subst $(1:/=),,$(2:/=)),$(findstring $(1:/=),$2),)
endef

# List sub-directories recursively for a list of root directories
# Results are sorted and without trailing path separator
# Sub-directories with spaces are skipped
# $1 -> Root paths
define ListAllSubDirs
$(foreach d,$(dir $(wildcard $1/*/.)),$(if $(call IsSubDir,$1,$d),$(d:/=) $(call ListAllSubDirs,$(d:/=))))
endef

# Display variable and list values, e.g. $(call PrintVariable,LIBS)
# $1 -> Name of variable containing values
define PrintVariable
	$(info $1)
	$(foreach item,$($1),$(info - $(item)))
endef

define PrintVariableSorted
	$(info $1)
	$(foreach item,$(sort $($1)),$(info - $(value item)))
endef

# Display list of variable references with their values e.g. $(call PrintVariableRefs,DEBUG_VARS)
# $1 -> Name of variable containing list of variable names
define PrintVariableRefs
	$(info $1)
	$(foreach item,$(sort $($1)),$(info - $(item) = $(value $(item))) )
endef

# Extract commented target information from makefiles and display
# Based on code from https://suva.sh/posts/well-documented-makefiles/
define PrintHelp
	@echo
	@echo Welcome to the Sming build system!
	@$(AWK)	'BEGIN { \
				FS = "(:.*##)|(##@)"; \
				printf "Usage:\n  make \033[1;36m<target>\033[0m\n"; \
			} /^##@/ { \
				group = $$2; \
				groups[group] = group; \
			} /^[a-zA-Z0-9_-]+:.*?##/ { \
				targets[$$1, group] = $$2; \
			} \
			END { \
				for (g in groups) { \
					printf "\n\033[1m%s\033[0m\n", g; \
					for (t in targets) { \
						split(t, sep, SUBSEP); \
						if (sep[2] == g) \
							printf "  \033[1;36m%-20s\033[0m %s\n", sep[1], targets[t] \
					} \
				} \
			} ' $(foreach f,$(MAKEFILE_LIST),"$(f)")
	@echo
endef


# Give relative or absolute source paths, convert them all to absolute
# $1 -> source root directory
# $2 -> file path(s)
define AbsoluteSourcePath
$(foreach f,$2,$(abspath $(if $(filter /%,$f),$f,$1/$f)))
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

# Update and patch submodule
# Patch file is either in submodule parent directory itself or subdirectory .patches from there
%/.submodule:
	$(info )
	$(info Fetching submodule '$*' ...)
	$(Q) cd $(abspath $*/..) && (rm -rf $(*F); $(GIT) submodule update --init --force --recursive $(*F))
	$(Q) $(call TryApplyPatch,$*,$(*F).patch)
	$(Q) touch $@

