/*
 * ESP32 attribute definitions
 */

#pragma once

#include_next <esp_attr.h>

// http://stackoverflow.com/a/35441900
#define MACROQUOT(x) #x
#define MACROQUOTE(x) MACROQUOT(x)

#define STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define STORE_ATTR __attribute__((aligned(4)))

#define DMEM_ATTR __attribute__((section(".bss")))
#define SHMEM_ATTR

#define ICACHE_FLASH_SECTION ".text"

#undef ICACHE_FLASH_ATTR
#define ICACHE_FLASH_ATTR                                                                                              \
	__attribute__((section(ICACHE_FLASH_SECTION "." __FILE__ "." MACROQUOTE(__LINE__) "." MACROQUOTE(__COUNTER__))))

#define ICACHE_RODATA_SECTION ".rodata"

#undef ICACHE_RODATA_ATTR
#define ICACHE_RODATA_ATTR                                                                                             \
	__attribute__((section(ICACHE_RODATA_SECTION "." __FILE__ "." MACROQUOTE(__LINE__) "." MACROQUOTE(__COUNTER__))))

#define ICACHE_RAM_SECTION ".iram0.text"

#undef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR                                                                                                \
	__attribute__((section(ICACHE_RAM_SECTION "." __FILE__ "." MACROQUOTE(__LINE__) "." MACROQUOTE(__COUNTER__))))

#define STORE_ATTR __attribute__((aligned(4)))

#ifdef ENABLE_GDB
#define GDB_IRAM_ATTR IRAM_ATTR
#else
#define GDB_IRAM_ATTR
#endif
