#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t system_get_free_heap_size(void);

#define os_malloc(s) malloc(s)
#define os_free(p) free(p)

#ifdef __cplusplus
}
#endif
