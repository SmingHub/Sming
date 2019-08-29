// ESP8266 attribute definitions (previously in c_types.h)
#pragma once

// http://stackoverflow.com/a/35441900
#define MACROQUOT(x) #x
#define MACROQUOTE(x) MACROQUOT(x)

#define STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define STORE_ATTR __attribute__((aligned(4)))

#define DMEM_ATTR __attribute__((section(".bss")))
#define SHMEM_ATTR

#define ICACHE_FLASH_ATTR \
	__attribute__((section(".irom0.text." __FILE__ MACROQUOTE(__LINE__) MACROQUOTE(__COUNTER__))))
#define ICACHE_RAM_ATTR \
	__attribute__((section(".iram.text." __FILE__ MACROQUOTE(__LINE__) MACROQUOTE(__COUNTER__))))
#define ICACHE_RODATA_ATTR \
	__attribute__((section(".irom0.text." __FILE__ MACROQUOTE(__LINE__) MACROQUOTE(__COUNTER__))))

#define IRAM_ATTR ICACHE_RAM_ATTR

#define STORE_ATTR __attribute__((aligned(4)))

#ifdef ENABLE_GDB
#define GDB_IRAM_ATTR IRAM_ATTR
#else
#define GDB_IRAM_ATTR
#endif
