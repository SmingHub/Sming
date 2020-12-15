COMPONENT_RELINK_VARS += ENABLE_MALLOC_COUNT

ENABLE_MALLOC_COUNT ?= 1

ifeq ($(ENABLE_MALLOC_COUNT),1)

COMPONENT_CXXFLAGS += -DENABLE_MALLOC_COUNT=1

# Hook all the memory allocation functions we need to monitor heap activity
EXTRA_LDFLAGS := \
	-Wl,-wrap,malloc \
	-Wl,-wrap,calloc \
	-Wl,-wrap,realloc \
	-Wl,-wrap,free

ifeq ($(SMING_ARCH),Esp8266)
EXTRA_LDFLAGS += \
	-Wl,-wrap,pvPortMalloc \
	-Wl,-wrap,pvPortCalloc \
	-Wl,-wrap,pvPortRealloc \
	-Wl,-wrap,pvPortZalloc \
	-Wl,-wrap,pvPortZallocIram \
	-Wl,-wrap,vPortFree
else
EXTRA_LDFLAGS += \
	-Wl,-wrap,strdup
endif

endif