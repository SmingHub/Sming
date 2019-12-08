COMPONENT_SRCDIRS		:=
COMPONENT_INCDIRS		:= include
COMPONENT_SRCFILES		:= libc.c

ifeq ($(USE_NEWLIB),1)
COMPONENT_SRCFILES		+= libc_replacements.c
LIBDIRS					+= $(COMPONENT_PATH)/newlib
EXTRA_LIBS				+= m c gcc
else
COMPONENT_SRCFILES		+= pgmspace.c
LIBDIRS					+= $(COMPONENT_PATH)/lib
EXTRA_LIBS				+= microc microgcc setjmp
endif
