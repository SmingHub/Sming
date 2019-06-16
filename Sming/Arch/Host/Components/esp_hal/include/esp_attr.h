#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IRAM_ATTR
#define STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_FLASH_ATTR

#define GDB_IRAM_ATTR

// Weak attributes don't work for PE
#ifdef __WIN32
#undef WEAK_ATTR
#define WEAK_ATTR
#endif

#ifdef __cplusplus
}
#endif
