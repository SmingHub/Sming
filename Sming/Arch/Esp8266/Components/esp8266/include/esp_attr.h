// ESP8266 attribute definitions (previously in c_types.h)
#pragma once

#define STORE_TYPEDEF_ATTR __attribute__((aligned(4), packed))
#define STORE_ATTR __attribute__((aligned(4)))

#define DMEM_ATTR __attribute__((section(".bss")))
#define SHMEM_ATTR

// The following two macros cause a parameter to be enclosed in quotes
#define ICACHE_RAM_ATTR __attribute__((section(".iram.text")))
// by the preopressor (i.e. for concatenating ints to strings)
#define ICACHE_RODATA_ATTR __attribute__((section(".irom.text")))
#define __ICACHE_STRINGIZE_NX(A) #A
#define ATTR_STRINGIZE__(A) __ICACHE_STRINGIZE_NX(A)
#define ICACHE_FLASH_ATTR                                                                                              \
	__attribute__((                                                                                                    \
		section("\".irom0.text." __FILE__ "." ATTR_STRINGIZE__(__LINE__) "." ATTR_STRINGIZE__(__COUNTER__) "\"")))
#define IRAM_ATTR                                                                                                      \
	__attribute__(                                                                                                     \
		(section("\".iram.text." __FILE__ "." ATTR_STRINGIZE__(__LINE__) "." ATTR_STRINGIZE__(__COUNTER__) "\"")))
#define ICACHE_RODATA_ATTR                                                                                             \
	__attribute__(                                                                                                     \
		(section("\".irom.text." __FILE__ "." ATTR_STRINGIZE__(__LINE__) "." ATTR_STRINGIZE__(__COUNTER__) "\"")))

#define STORE_ATTR __attribute__((aligned(4)))

#ifdef ENABLE_GDB
#define GDB_IRAM_ATTR IRAM_ATTR
#else
#define GDB_IRAM_ATTR
#endif
