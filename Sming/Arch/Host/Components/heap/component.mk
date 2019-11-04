# We require malloc_count to keep track of heap usage for system_get_free_heap_size()
# If using valgrind, for example, disabling hooking provides a cleaner trace
COMPONENT_VARS			:= ENABLE_MALLOC_COUNT
ENABLE_MALLOC_COUNT		?= 1

ifeq ($(ENABLE_MALLOC_COUNT),1)
GLOBAL_CFLAGS			+= -DENABLE_MALLOC_COUNT
COMPONENT_DEPENDS		:= malloc_count
endif
