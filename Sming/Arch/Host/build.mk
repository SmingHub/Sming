##############
#
# Host Platform build environment
#
##############

CPPFLAGS	+= -DARCH_HOST

TOOLSPEC 	:=

AS		:= $(TOOLSPEC)gcc
CC		:= $(TOOLSPEC)gcc
CXX		:= $(TOOLSPEC)g++
AR		:= $(TOOLSPEC)ar
LD		:= $(TOOLSPEC)g++
NM		:= $(TOOLSPEC)nm
OBJCOPY		:= $(TOOLSPEC)objcopy
OBJDUMP		:= $(TOOLSPEC)objdump
GDB		:= $(TOOLSPEC)gdb

GCC_UPGRADE_URL := https://sming.readthedocs.io/en/latest/arch/host/host-emulator.html\#c-c-32-bit-compiler-and-libraries

CPPFLAGS += \
	-m32 \
	-Wno-deprecated-declarations \
	-D_FILE_OFFSET_BITS=64

# => Tools
MEMANALYZER = size

# Command-line options passed to executable - Components add their own settings to this
CLI_TARGET_OPTIONS =
