# Build environment definitions

# Use this form to define platform as environment variable
ifdef SMING_PLATFORM
PLATFORM		:= $(SMING_PLATFORM)
else
PLATFORM		?= Esp8266
endif

MAKECMDGOALS ?= all
$(info Building '$(MAKECMDGOALS)' for '$(PLATFORM)' platform)

LIBS			?=
CUSTOM_TARGETS	?=
CLEAN			?=
TOOLS			?=
TOOLS_CLEAN		?=

# Detect OS and build environment
UNAME := $(shell uname -s)

ifeq ($(OS),Windows_NT)
  # Convert Windows paths to POSIX paths
  SMING_HOME := $(subst \,/,$(addprefix /,$(subst :,,$(SMING_HOME))))
  SMING_HOME := $(subst //,/,$(SMING_HOME))
endif

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

PLATFORM_BASE		:= Platform/$(PLATFORM)
PLATFORM_SYS		= $(PLATFORM_BASE)/System
PLATFORM_CORE		= $(PLATFORM_BASE)/Core
PLATFORM_TOOLS		= $(PLATFORM_BASE)/Tools
PLATFORM_COMPONENTS	= $(PLATFORM_SYS)/components
USER_LIBDIR			= $(PLATFORM_BASE)/Compiler/lib
THIRD_PARTY_DIR		= third-party

# Git command
GIT ?= git

### Debug output parameters
# By default `debugf` does not print file name and line number. If you want this enabled set the directive below to 1
DEBUG_PRINT_FILENAME_AND_LINE ?= 0

# Default debug verbose level is INFO, where DEBUG=3 INFO=2 WARNING=1 ERROR=0
DEBUG_VERBOSE_LEVEL ?= 2

# Disable CommandExecutor functionality if not used and save some ROM and RAM
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
ifneq ($(STRICT),1)
CFLAGS += -Werror -Wno-sign-compare -Wno-parentheses -Wno-unused-variable -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-return-type -Wno-maybe-uninitialized
endif

ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -ggdb -DENABLE_GDB=1
endif

ifeq ($(SMING_RELEASE),1)
	# See: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	#      for full list of optimization options
	CFLAGS += -Os -DSMING_RELEASE=1 -DLWIP_NOASSERT
else ifeq ($(ENABLE_GDB), 1)
	CFLAGS += -Og
else
	CFLAGS += -Os -g
endif

#Append debug options
CFLAGS += -DCUST_FILE_BASE=$$* -DDEBUG_VERBOSE_LEVEL=$(DEBUG_VERBOSE_LEVEL) -DDEBUG_PRINT_FILENAME_AND_LINE=$(DEBUG_PRINT_FILENAME_AND_LINE)

CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11 -felide-constructors
ifneq ($(STRICT),1)
CXXFLAGS += -Wno-reorder
endif

# => LOCALE
ifdef LOCALE
	CFLAGS += -DLOCALE=$(LOCALE)
endif

include $(SMING_HOME)/$(PLATFORM_BASE)/build.mk

AS	:= $(Q)$(AS)
CC	:= $(Q)$(CC)
CXX	:= $(Q)$(CXX)
AR	:= $(Q)$(AR)
LD	:= $(Q)$(LD)

# Declare target for user library
# $1 -> Name of library
define UserLibPath
	$(USER_LIBDIR)/lib$1.a
endef

