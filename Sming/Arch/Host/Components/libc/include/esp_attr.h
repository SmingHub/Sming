#pragma once
#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// http://stackoverflow.com/a/35441900
#define MACROQUOT(x) #x
#define MACROQUOTE(x) MACROQUOT(x)

#define ICACHE_FLASH_SECTION ".text"
#define ICACHE_RODATA_SECTION ".rodata"
#define ICACHE_RAM_SECTION ".data"

#define IRAM_ATTR
#define STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_FLASH_ATTR

#define GDB_IRAM_ATTR

#ifdef __cplusplus
}
#endif
