#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Frequencies in MHz */

#define SYS_CPU_80MHZ 80
#define SYS_CPU_160MHZ 160

bool system_update_cpu_freq(uint8 freq);
uint32_t ets_get_cpu_frequency(void);
uint32_t system_get_cpu_freq(void);

/* Emulation of CPU cycle count */
uint32_t esp_get_ccount();

#ifdef __cplusplus
}
#endif
