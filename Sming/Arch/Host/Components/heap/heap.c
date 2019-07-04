
#include "include/heap.h"
#include "malloc_count.h"

// Notional RAM available
const uint32_t memorySize = 128 * 1024;

uint32_t system_get_free_heap_size(void)
{
	uint32_t current = malloc_count_current();
	return (current < memorySize) ? (memorySize - current) : 0;
}
