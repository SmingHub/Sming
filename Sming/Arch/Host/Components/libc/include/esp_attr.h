#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IRAM_ATTR
#define STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_FLASH_ATTR

#define PROGMEM STORE_ATTR
#define PROGMEM_PSTR PROGMEM

#define GDB_IRAM_ATTR

#ifdef __cplusplus
}
#endif
