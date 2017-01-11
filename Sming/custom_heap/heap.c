/* heap.c - overrides of SDK heap handling functions
 * Copyright (c) 2016 Ivan Grokhotkov. All rights reserved.
 * This file is distributed under MIT license.
 */

#include <stdlib.h>
#include <c_types.h>
#include "umm_malloc_cfg.h"
#include "umm_malloc.h"

#define IRAM_ATTR __attribute__((section(".iram.text")))

void* IRAM_ATTR pvPortMalloc(size_t size, const char* file, int line)
{
    return malloc(size);
}

void IRAM_ATTR vPortFree(void *ptr, const char* file, int line)
{
    free(ptr);
}

void* IRAM_ATTR pvPortCalloc(size_t count, size_t size, const char* file, int line)
{
    return calloc(count, size);
}

void* IRAM_ATTR pvPortRealloc(void *ptr, size_t size, const char* file, int line)
{
    return realloc(ptr, size);
}

void* IRAM_ATTR pvPortZalloc(size_t size, const char* file, int line)
{
    return calloc(1, size);
}

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
