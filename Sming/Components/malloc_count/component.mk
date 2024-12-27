COMPONENT_RELINK_VARS += ENABLE_MALLOC_COUNT
COMPONENT_DOXYGEN_INPUT := include

# Not currently supported on MacOS
ifeq ($(UNAME),Darwin)
override ENABLE_MALLOC_COUNT=0
else
ENABLE_MALLOC_COUNT ?= 1
endif

ifeq ($(ENABLE_MALLOC_COUNT),1)

COMPONENT_CXXFLAGS += -DENABLE_MALLOC_COUNT=1

# Hook all the memory allocation functions we need to monitor heap activity
MC_WRAP_FUNCS := \
	malloc \
	calloc \
	realloc \
	free \
	strdup
ifeq ($(SMING_ARCH),Esp8266)
MC_WRAP_FUNCS += \
	pvPortMalloc \
	pvPortCalloc \
	pvPortRealloc \
	pvPortZalloc \
	pvPortZallocIram \
	vPortFree
endif
ifeq ($(SMING_ARCH)$(UNAME),HostWindows)
MC_WRAP_FUNCS += \
	__mingw_realloc \
	__mingw_free
endif

EXTRA_LDFLAGS := $(call UndefWrap,$(MC_WRAP_FUNCS))

endif
