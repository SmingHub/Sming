COMPONENT_INCDIRS := src/include
COMPONENT_SRCDIRS := src

# pico_printf
LIBC_DEFSYMS := \
	__wrap_putchar=m_putc \
	__wrap_puts=m_puts \
	__wrap_vprintf=m_vprintf \
	__wrap_printf=m_printf

EXTRA_LDFLAGS := $(call DefSym,$(LIBC_DEFSYMS))
