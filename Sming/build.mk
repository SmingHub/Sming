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
ARCH_LIBDIR		= $(ARCH_BASE)/Compiler/lib

OUT_BASE		:= out/$(SMING_ARCH)/$(BUILD_TYPE)
BUILD_BASE		= $(OUT_BASE)/build
FW_BASE			= $(OUT_BASE)/firmware
TOOLS_BASE		= $(SMING_HOME)/$(OUT_BASE)/tools
USER_LIBDIR		= $(SMING_HOME)/$(OUT_BASE)/lib

# Git command
GIT ?= git

# CMake command
CMAKE ?= cmake


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
	CFLAGS		+= -Os -DSMING_RELEASE=1
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS		+= -Og
else
	CFLAGS		+= -Os -g
endif

CXXFLAGS = $(CFLAGS) -std=c++11 -felide-constructors

ifneq ($(STRICT),1)
	CXXFLAGS += -Wno-reorder
endif

include $(ARCH_BASE)/build.mk

# Component (user) libraries have a special prefix so linker script can identify them
CLIB_PREFIX := clib-

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

# List sub-directories recursively for a list of root directories
# Results are sorted and without trailing path separator
# $1 -> Root paths
define ListAllSubDirs
$(foreach d,$(dir $(wildcard $1/*/.)),$(d:/=) $(call ListAllSubDirs,$(d:/=)))
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
	@awk	'BEGIN { \
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
			} ' $(MAKEFILE_LIST)
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
		cp -f ../.patches/$(basename $2)/* . ; \
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

