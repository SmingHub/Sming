COMPONENT_SRCDIRS		:=
COMPONENT_INCDIRS		:= include
COMPONENT_SRCFILES		:= libc.c

ifeq ($(USE_NEWLIB),1)
COMPONENT_SRCFILES		+= libc_replacements.c
endif
