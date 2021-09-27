#pragma once

#include <c_types.h>
#include <hal/cpu_hal.h>
#include <sming_attr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Frequencies in MHz */

#define SYS_CPU_80MHZ 80
#define SYS_CPU_160MHZ 160
#define SYS_CPU_240MHZ 240

bool system_update_cpu_freq(uint32_t freq);
uint32_t system_get_cpu_freq(void);

__forceinline static uint32_t esp_get_ccount()
{
	return cpu_hal_get_cycle_count();
}

#ifdef __cplusplus
}
#endif
