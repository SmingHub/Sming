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
OUT_BASE	:= out/$(SMING_ARCH)/$(UNAME)/$(if $(SMING_RELEASE),release,debug)

# => Tools
MEMANALYZER	= size
