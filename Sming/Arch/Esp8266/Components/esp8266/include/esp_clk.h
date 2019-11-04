#pragma once

#include <stdint.h>
#include <sming_attr.h>

#ifdef __cplusplus
extern "C" {
#endif

// system_get_cpu_frequency is just a wrapper for this ROM function.
uint8_t ets_get_cpu_frequency(void);

__forceinline static uint32_t esp_get_ccount()
{
	uint32_t ccount;
	__asm__ __volatile__("rsr %0, ccount\n" : "=a"(ccount) : : "memory");
	return ccount;
}

#ifdef __cplusplus
}
#endif
