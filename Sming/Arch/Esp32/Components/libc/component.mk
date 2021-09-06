COMPONENT_SRCDIRS		:= src
COMPONENT_INCDIRS		:= src/include

COMPONENT_DOXYGEN_INPUT	:= src/include/sys

EXTRA_LDFLAGS := \
	-Wl,-wrap,_write_r \
	-Wl,-wrap,_read_r \
