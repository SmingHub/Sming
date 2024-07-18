COMPONENT_INCDIRS := src/include
COMPONENT_SRCDIRS := src

# pico_printf
LIBC_DEFSYMS := \
	__wrap_putchar=m_putc \
	__wrap_puts=m_puts \
	__wrap_vprintf=m_vprintf \
	__wrap_printf=m_printf

LIBC_UNDEFSYMS := \
	_close_r \
	_lseek_r \
	_read_r \
	_write_r \
	_isatty_r


EXTRA_LDFLAGS := \
	$(call DefSym,$(LIBC_DEFSYMS)) \
	$(call Undef,$(LIBC_UNDEFSYMS))
