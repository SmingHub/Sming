# Build environment definitions

# Add configuration variable names to CONFIG_VARS as they are encountered (see ListConfig macro)
CONFIG_VARS :=

#
CONFIG_VARS 	+= SMING_ARCH
SMING_ARCH		?= Esp8266

MAKECMDGOALS	?= all
$(info Invoking '$(MAKECMDGOALS)' for '$(SMING_ARCH)' architecture)

LIBS			?=
CUSTOM_TARGETS	?=
CLEAN			?=
TOOLS			?=
TOOLS_CLEAN		?=

# Detect OS and build environment
UNAME := $(shell uname -s)

CONFIG_VARS += UNAME
ifneq ($(filter MINGW32_NT%,$(UNAME)),)
  UNAME := Windows
else ifneq ($(filter CYGWIN%,$(UNAME)),)
  # Cygwin Detected
  UNAME := Linux
else ifneq ($(filter CYGWIN%WOW,$(UNAME)),)
  #Cygwin32
  UNAME := Linux
else ifneq ($(filter MSYS%WOW,$(UNAME)),)
  #Msys32
  UNAME := Linux
else ifeq ($(UNAME), Linux)
  #Linux
else ifeq ($(UNAME), Darwin)
  #OS X
else ifeq ($(UNAME), Freebsd)
  #BSD
endif


# Convert Windows paths to POSIX paths
CONFIG_VARS += OS
ifeq ($(OS),Windows_NT)
FixPath = $(subst //,/,$(subst \,/,$(addprefix /,$(subst :,,$1))))
else
FixPath = $1
endif

CONFIG_VARS	+= SMING_HOME
SMING_HOME	:= $(call FixPath,$(realpath $(SMING_HOME)))

# OS specific configuration
ifeq ($(UNAME),Windows)
  # Windows detected
  include $(SMING_HOME)/Makefile-windows.mk
  TOOL_EXT := .exe
else
  ifeq ($(UNAME),Darwin)
      # MacOS Detected
      UNAME := MacOS
      include $(SMING_HOME)/Makefile-macos.mk
  else ifeq ($(UNAME),Linux)
      # Linux Detected
      include $(SMING_HOME)/Makefile-linux.mk
  else ifeq ($(UNAME),FreeBSD)
      # Freebsd Detected
      include $(SMING_HOME)/Makefile-bsd.mk
  endif
endif

export SMING_HOME
export COMPILE := gcc

CONFIG_VARS		+= ARCH_BASE USER_LIBDIR BUILD_BASE FW_BASE

ARCH_BASE		:= Arch/$(SMING_ARCH)
ARCH_SYS		= $(ARCH_BASE)/System
ARCH_CORE		= $(ARCH_BASE)/Core
ARCH_TOOLS		= $(ARCH_BASE)/Tools
ARCH_COMPONENTS	= $(ARCH_BASE)/Components
USER_LIBDIR		= $(ARCH_BASE)/Compiler/lib
COMPONENTS		:= Components

BUILD_BASE		:= out/build/$(SMING_ARCH)
FW_BASE			:= out/firmware

# Git command
GIT ?= git

# CMake command
CMAKE ?= cmake

### Debug output parameters
# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
CONFIG_VARS += DEBUG_PRINT_FILENAME_AND_LINE
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

# Default debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0
CONFIG_VARS += DEBUG_VERBOSE_LEVEL
DEBUG_VERBOSE_LEVEL ?= 2

# Disable CommandExecutor functionality if not used and save some ROM and RAM
CONFIG_VARS += ENABLE_CMD_EXECUTOR
ENABLE_CMD_EXECUTOR ?= 1

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

# Common flags passed to user libraries
CFLAGS_COMMON	= -Wl,-EL -finline-functions -fdata-sections -ffunction-sections
# compiler flags using during compilation of source files. Add '-pg' for debugging
CFLAGS			= -Wall -Wundef -Wpointer-arith -Wno-comment $(CFLAGS_COMMON) \
         			-DARDUINO=106 -DENABLE_CMD_EXECUTOR=$(ENABLE_CMD_EXECUTOR) -DSMING_INCLUDED=1
CONFIG_VARS += STRICT
ifneq ($(STRICT),1)
	CFLAGS += -Werror -Wno-sign-compare -Wno-parentheses -Wno-unused-variable -Wno-unused-but-set-variable -Wno-strict-aliasing
endif

CONFIG_VARS += ENABLE_GDB
ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -ggdb -DENABLE_GDB=1
endif

CONFIG_VARS += SMING_RELEASE
ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	CFLAGS += -Os -DSMING_RELEASE=1 -DLWIP_NOASSERT
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -Og
else
	CFLAGS += -Os -g
endif

#
CONFIG_VARS	+= USER_CFLAGS
CFLAGS		+= $(USER_CFLAGS)

#Append debug options
CONFIG_VARS += SMING_RELEASE
CFLAGS += -DCUST_FILE_BASE=$$* -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)

CXXFLAGS = $(CFLAGS) -std=c++11 -felide-constructors
ifneq ($(STRICT),1)
	CXXFLAGS += -Wno-reorder
endif

# => LOCALE
ifdef LOCALE
	CFLAGS += -DLOCALE=$(LOCALE)
endif

include $(SMING_HOME)/$(ARCH_BASE)/build.mk

# Declare target for user library
# $1 -> Name of library
define UserLibPath
	$(USER_LIBDIR)/lib$1.a
endef

# Fetch full path for submodules matching given pattern
# $1 -> Path pattern to match
define ListSubmodules
	$(shell git submodule status $1 | cut -c2- | cut -f2 -d ' ')
endef

# Display variable and list values, e.g. $(call PrintVariable,LIBS)
# $1 -> Name of variable containing values
define PrintVariable
	$(info $1)
	$(foreach item,$(sort $($1)),$(info - $(item)))
endef

# Display list of variable references with their values e.g. $(call PrintVariableRefs,CONFIG_VARS)
# $1 -> Name of variable containing list of variable names
define PrintVariableRefs
	$(info $1)
	$(foreach item,$(sort $($1)),$(info - $(item) = $($(item))) )
endef

# Display some internal configuration information
define ListConfig
	$(info )
	$(info ** Sming internal build configuration **)
	$(info )
	$(call PrintVariable,MAKEFILE_LIST)
	$(call PrintVariable,CUSTOM_TARGETS)
	$(call PrintVariable,LIBSMING)
	$(call PrintVariable,LIBS)
	$(call PrintVariable,ARDUINO_LIBRARIES)
	$(call PrintVariable,SUBMODULES)
	$(call PrintVariableRefs,CONFIG_VARS)
	$(info )
	$(info )
endef

# Extract commented target information from makefiles and display
# Based on code from https://suva.sh/posts/well-documented-makefiles/
define PrintHelp
	$(info )
	$(info Welcome to the Sming build system!)
	@awk	'BEGIN { \
				FS = "(:.*##)|(##@)"; \
				printf "Usage:\n  make \033[1;36m<target>\033[0m\n"; \
			} /^##@/ { \
				group = $$2; \
				groups[group] = group; \
			} /^[a-zA-Z_-]+:.*?##/ { \
				targets[$$1, group] = $$2; \
			} \
			END { \
				for (g in groups) { \
					printf "\n\033[1m%s\033[0m\n", g; \
					for (t in targets) { \
						split(t, sep, SUBSEP); \
						if (sep[2] == g) \
							printf "  \033[1;36m%-18s\033[0m %s\n", sep[1], targets[t] \
					} \
				} \
			} ' $(MAKEFILE_LIST)
	$(info )
endef


# => Main Sming library
CONFIG_VARS += ENABLE_SSL
ifeq ($(ENABLE_SSL),1)
	LIBSMING		= smingssl
	SMING_FEATURES	= SSL
else
	LIBSMING		= sming
	SMING_FEATURES	= none
endif
LIBSMING_DST 		= $(call UserLibPath,$(LIBSMING))

# => MQTT
# Flags for compatability with old versions (most of them should disappear with the next major release)
CONFIG_VARS += MQTT_NO_COMPAT
ifeq ($(MQTT_NO_COMPAT),1)
	CFLAGS	+= -DMQTT_NO_COMPAT=1
endif
