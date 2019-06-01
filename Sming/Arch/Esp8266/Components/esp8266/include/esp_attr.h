// ESP8266 attribute definitions (previously in c_types.h)
#pragma once

#define IRAM_ATTR __attribute__((section(".iram.text")))
#define STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define STORE_ATTR __attribute__((aligned(4)))

#define DMEM_ATTR __attribute__((section(".bss")))
#define SHMEM_ATTR

#ifdef ICACHE_FLASH
#define ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#define ICACHE_RODATA_ATTR __attribute__((section(".irom.text")))
#else
#define ICACHE_FLASH_ATTR
#define ICACHE_RODATA_ATTR
#endif

#define STORE_ATTR __attribute__((aligned(4)))

#ifdef ENABLE_GDB
	#define GDB_IRAM_ATTR IRAM_ATTR
#else
	#define GDB_IRAM_ATTR
#endif
