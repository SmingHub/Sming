##############
#
# Host Platform build environment
#
##############

CPPFLAGS	+= -DARCH_HOST

TOOLSPEC 	:=

ifndef CLANG_BUILD
override CLANG_BUILD := 0
endif

ifneq ($(CLANG_BUILD),0)
ifeq ($(CLANG_BUILD),1)
CLANG_VER :=
else
CLANG_VER := -$(CLANG_BUILD)
endif
AS		:= $(TOOLSPEC)clang$(CLANG_VER)
CC		:= $(TOOLSPEC)clang$(CLANG_VER)
CXX		:= $(TOOLSPEC)clang++$(CLANG_VER)
AR		:= $(TOOLSPEC)ar
LD		:= $(TOOLSPEC)clang++$(CLANG_VER)
NM		:= $(TOOLSPEC)nm
OBJCOPY		:= $(TOOLSPEC)objcopy
OBJDUMP		:= $(TOOLSPEC)objdump
else
AS		:= $(TOOLSPEC)gcc
CC		:= $(TOOLSPEC)gcc
CXX		:= $(TOOLSPEC)g++
AR		:= $(TOOLSPEC)ar
LD		:= $(TOOLSPEC)g++
NM		:= $(TOOLSPEC)nm
OBJCOPY		:= $(TOOLSPEC)objcopy
OBJDUMP		:= $(TOOLSPEC)objdump
endif

GDB		:= $(TOOLSPEC)gdb

GCC_UPGRADE_URL := https://sming.readthedocs.io/en/latest/arch/host/host-emulator.html\#c-c-32-bit-compiler-and-libraries

ifeq ($(UNAME),Darwin)
BUILD64 := 1
CPPFLAGS += -D_DARWIN_C_SOURCE=1
endif

ifneq ($(BUILD64),1)
CPPFLAGS += -m32
endif

CPPFLAGS += \
	-D_FILE_OFFSET_BITS=64 \
	-D_TIME_BITS=64

# => Tools
MEMANALYZER = size

# Command-line options passed to executable - Components add their own settings to this
CLI_TARGET_OPTIONS =
