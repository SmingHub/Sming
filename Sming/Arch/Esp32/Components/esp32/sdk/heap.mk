#
# heap
#

ifdef CONFIG_HEAP_TRACING
SDK_WRAP_SYMBOLS += \
	calloc \
	malloc \
	free \
	realloc \
	heap_caps_malloc \
	heap_caps_free \
	heap_caps_realloc \
	heap_caps_malloc_default \
	heap_caps_realloc_default
endif
