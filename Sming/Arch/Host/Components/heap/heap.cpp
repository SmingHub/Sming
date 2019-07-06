
#include "include/heap.h"
#ifdef ENABLE_MALLOC_COUNT
#include <malloc_count.h>
#endif

// Notional RAM available
const uint32_t memorySize = 128 * 1024;

uint32_t system_get_free_heap_size(void)
{
#ifdef ENABLE_MALLOC_COUNT
	uint32_t current = MallocCount::getCurrent();
	return (current < memorySize) ? (memorySize - current) : 0;
#else
	return memorySize;
#endif
}
