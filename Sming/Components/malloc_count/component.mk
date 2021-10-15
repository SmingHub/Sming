COMPONENT_RELINK_VARS += ENABLE_MALLOC_COUNT

ENABLE_MALLOC_COUNT ?= 1

ifeq ($(ENABLE_MALLOC_COUNT),1)

COMPONENT_CXXFLAGS += -DENABLE_MALLOC_COUNT=1

# Hook all the memory allocation functions we need to monitor heap activity
ifeq ($(SMING_ARCH),Esp8266)
MC_WRAP_FUNCS := \
	pvPortMalloc \
	pvPortCalloc \
	pvPortRealloc \
	pvPortZalloc \
	pvPortZallocIram \
	vPortFree
else
MC_WRAP_FUNCS := \
	malloc \
	calloc \
	realloc \
	free \
	strdup
endif

EXTRA_LDFLAGS := $(call UndefWrap,$(MC_WRAP_FUNCS))

endif
