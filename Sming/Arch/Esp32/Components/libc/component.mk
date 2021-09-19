COMPONENT_SRCDIRS		:= src
COMPONENT_INCDIRS		:= src/include

COMPONENT_DOXYGEN_INPUT	:= src/include/sys

EXTRA_LDFLAGS := $(call Wrap,_write_r _read_r)
