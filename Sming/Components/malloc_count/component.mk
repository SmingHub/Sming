# Hook all the memory allocation functions we need to monitor heap activity
ifeq ($(SMING_ARCH),Host)
EXTRA_LDFLAGS := \
	-Wl,-wrap,malloc \
	-Wl,-wrap,calloc \
	-Wl,-wrap,realloc \
	-Wl,-wrap,free
else
EXTRA_LDFLAGS := \
	-Wl,-wrap,pvPortMalloc \
	-Wl,-wrap,pvPortCalloc \
	-Wl,-wrap,pvPortRealloc \
	-Wl,-wrap,pvPortZalloc \
	-Wl,-wrap,pvPortZallocIram \
	-Wl,-wrap,vPortFree
endif
