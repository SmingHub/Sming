##############
#
# Host Platform build environment
#
##############

CFLAGS += -DARCH_HOST

TOOLSPEC :=

AS		:= $(TOOLSPEC)gcc
CC		:= $(TOOLSPEC)gcc
CXX		:= $(TOOLSPEC)g++
AR		:= $(TOOLSPEC)ar
LD		:= $(TOOLSPEC)g++
OBJCOPY := $(TOOLSPEC)objcopy
OBJDUMP := $(TOOLSPEC)objdump
GDB		:= $(TOOLSPEC)gdb

CFLAGS	+= -m32 -Wno-deprecated-declarations

# Keep Windows/Linux object files separate to avoid conflict
BUILD_BASE	:= $(BUILD_BASE)/$(UNAME)
USER_LIBDIR	= $(ARCH_BASE)/Compiler/lib/$(UNAME)

# => Tools
SPIFFY		= $(ARCH_BASE)/../Esp8266/Tools/spiffy/spiffy$(TOOL_EXT)
MEMANALYZER	= size

Terminal = start telnet localhost $$((10000 + $1))

TERMINAL = $(call Terminal,$(COM_PORT))
KILL_TERM :=
