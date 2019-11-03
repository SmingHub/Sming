/* heap.c - overrides of SDK heap handling functions
 * Copyright (c) 2016 Ivan Grokhotkov. All rights reserved.
 * This file is distributed under MIT license.
 */

#include <stdlib.h>
#include <c_types.h>
#include "umm_malloc_cfg.h"
#include "umm_malloc.h"

#define IRAM_ATTR __attribute__((section(".iram.text")))

#ifdef UMM_POISON_CHECK
#define UMM_FUNC(f) umm_poison_##f
#else
#define UMM_FUNC(f) umm_##f
#endif


void* IRAM_ATTR pvPortMalloc(size_t size, const char* file, int line)
{
    return UMM_FUNC(malloc)(size);
}

void IRAM_ATTR vPortFree(void *ptr, const char* file, int line)
{
	UMM_FUNC(free)(ptr);
}

void* IRAM_ATTR pvPortCalloc(size_t count, size_t size, const char* file, int line)
{
    return UMM_FUNC(calloc)(count, size);
}

void* IRAM_ATTR pvPortRealloc(void *ptr, size_t size, const char* file, int line)
{
    return UMM_FUNC(realloc)(ptr, size);
}

void* IRAM_ATTR pvPortZalloc(size_t size, const char* file, int line)
{
    return UMM_FUNC(calloc)(1, size);
}

void* IRAM_ATTR pvPortZallocIram(size_t size, const char* file, int line) __attribute__ ((weak, alias("pvPortZalloc")));

size_t xPortGetFreeHeapSize(void)
{
    return umm_free_heap_size();
}

size_t IRAM_ATTR xPortWantedSizeAlign(size_t size)
{
    return (size + 3) & ~((size_t) 3);
}

void system_show_malloc(void)
{
    umm_info(NULL, 1);
}
