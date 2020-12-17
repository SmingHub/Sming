COMPONENT_SRCDIRS		:=
COMPONENT_INCDIRS		:= include
COMPONENT_SRCDIRS		:= src

COMPONENT_DOXYGEN_INPUT	:= include/sys

ifeq ($(USE_NEWLIB),1)
COMPONENT_SRCDIRS		+= src/newlib
EXTRA_LIBS				+= m c gcc
else
COMPONENT_SRCDIRS		+= src/oldlib
LIBDIRS					+= $(COMPONENT_PATH)/lib
EXTRA_LIBS				+= microc microgcc setjmp
endif
