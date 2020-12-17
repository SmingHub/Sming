#pragma once

#include <c_types.h>
#include <xtensa/core-macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Frequencies in MHz */

#define SYS_CPU_80MHZ 80
#define SYS_CPU_160MHZ 160
#define SYS_CPU_240MHZ 240

__forceinline bool system_update_cpu_freq(uint8_t freq)
{
	if(freq != SYS_CPU_80MHZ && freq != SYS_CPU_160MHZ && freq != SYS_CPU_240MHZ) {
		return false;
	}

	ets_update_cpu_frequency(freq);
	return true;
}

__forceinline uint32_t system_get_cpu_freq(void)
{
	return ets_get_cpu_frequency();
}

__forceinline uint32_t esp_get_ccount()
{
	return XTHAL_GET_CCOUNT();
}

#ifdef __cplusplus
}
#endif
