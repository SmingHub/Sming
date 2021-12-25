COMPONENT_SRCDIRS		:= src
COMPONENT_INCDIRS		:= src/include

COMPONENT_DOXYGEN_INPUT	:= src/include/sys

LIBC_WRAPSYMS := \
	_write_r \
	_read_r \
	putchar \
	puts \
	vprintf \
	printf \
	vsnprintf \
	vsprintf \
	sprintf

EXTRA_LDFLAGS := $(call Wrap,$(LIBC_WRAPSYMS))

EXTRA_LIBS := \
	c \
	m  \
 	gcc \
	stdc++
