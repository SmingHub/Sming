/*
 * heap.c
 */

#include "include/heap.h"
#include <malloc.h>

uint32_t system_get_free_heap_size(void)
{
	// These are set by linker
	extern char __end__;
	extern char __StackLimit;
	uint32_t maxHeap = (uint32_t)&__StackLimit - (uint32_t)&__end__;
	struct mallinfo m = mallinfo();
	return maxHeap - m.uordblks;
}
