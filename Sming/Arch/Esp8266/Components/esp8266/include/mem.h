#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_INTERNAL

// Missing from early SDK versions
extern void *pvPortMalloc(size_t xWantedSize, const char *file, uint32 line);
extern void *pvPortZalloc(size_t xWantedSize, const char *file, uint32 line);
extern void pvPortFree(void *ptr);
extern void vPortFree(void *ptr, const char *file, uint32 line);
extern void *vPortMalloc(size_t xWantedSize);

#endif

#include_next <mem.h>

#ifdef __cplusplus
}
#endif

