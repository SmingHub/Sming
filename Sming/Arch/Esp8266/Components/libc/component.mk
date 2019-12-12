COMPONENT_SRCDIRS		:=
COMPONENT_INCDIRS		:= include
COMPONENT_SRCFILES		:= libc.c

COMPONENT_DOXYGEN_INPUT	:= include/sys

ifeq ($(USE_NEWLIB),1)
COMPONENT_SRCFILES		+= libc_replacements.c
EXTRA_LIBS				+= m c gcc
else
COMPONENT_SRCFILES		+= pgmspace.c
LIBDIRS					+= $(COMPONENT_PATH)/lib
EXTRA_LIBS				+= microc microgcc setjmp
endif
