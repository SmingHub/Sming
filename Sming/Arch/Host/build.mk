##############
#
# Host Platform build environment
#
##############

CFLAGS		+= -DARCH_HOST

TOOLSPEC 	:=

AS			:= $(TOOLSPEC)gcc
CC			:= $(TOOLSPEC)gcc
CXX			:= $(TOOLSPEC)g++
AR			:= $(TOOLSPEC)ar
LD			:= $(TOOLSPEC)g++
OBJCOPY		:= $(TOOLSPEC)objcopy
OBJDUMP		:= $(TOOLSPEC)objdump
GDB			:= $(TOOLSPEC)gdb

CFLAGS += \
	-m32 \
	-Wno-deprecated-declarations

# Keep Windows/Linux object files separate to avoid conflict
OUT_BASE	:= out/$(SMING_ARCH)/$(UNAME)/$(if $(SMING_RELEASE),release,debug)

# => Tools
MEMANALYZER = size

# Command-line options passed to executable - Components add their own settings to this
CLI_TARGET_OPTIONS =

# Run a command in a new terminal window
# $1 -> Command to execute
ifeq ($(UNAME),Windows)
DetachCommand = start $1
else
DetachCommand = gnome-terminal -- bash -c "sleep 1; $1"
endif
