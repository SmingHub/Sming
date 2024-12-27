/*
 * heap.c
 */

#include "include/heap.h"
#include <malloc.h>
#include <cstdlib>
#include <new>

extern "C" uint32_t system_get_free_heap_size(void)
{
	// These are set by linker
	extern char __end__;
	extern char __StackLimit;
	uint32_t maxHeap = (uint32_t)&__StackLimit - (uint32_t)&__end__;
	struct mallinfo m = mallinfo();
	return maxHeap - m.uordblks;
}

void* operator new(size_t size)
{
	return malloc(size);
}

void* operator new(size_t size, const std::nothrow_t&)
{
	return malloc(size);
}

void* operator new[](size_t size)
{
	return malloc(size);
}

void* operator new[](size_t size, const std::nothrow_t&)
{
	return malloc(size);
}

void operator delete(void* ptr)
{
	free(ptr);
}

void operator delete[](void* ptr)
{
	free(ptr);
}

void operator delete(void* ptr, size_t)
{
	free(ptr);
}

void operator delete[](void* ptr, size_t)
{
	free(ptr);
}
