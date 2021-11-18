#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void* pvPortMalloc(size_t xWantedSize, const char* file, uint32 line);
void* pvPortCalloc(size_t count, size_t size, const char*, unsigned);
void* pvPortZalloc(size_t xWantedSize, const char* file, uint32 line);
void vPortFree(void* ptr, const char* file, uint32 line);
void* vPortMalloc(size_t xWantedSize);
void pvPortFree(void* ptr);

#define os_malloc(s) pvPortMalloc(s, "", __LINE__)
#define os_calloc(l, s) pvPortCalloc(l, s, "", __LINE__)
#define os_realloc(p, s) pvPortRealloc(p, s, "", __LINE__)
#define os_zalloc(s) pvPortZalloc(s, "", __LINE__)
#define os_free(s) vPortFree(s, "", __LINE__)

#ifdef __cplusplus
}
#endif
