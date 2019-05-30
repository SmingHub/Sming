#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Frequencies in MHz */

#define SYS_CPU_80MHZ 80
#define SYS_CPU_160MHZ 160

bool system_update_cpu_freq(uint8 freq);
uint8 system_get_cpu_freq(void);

#ifdef __cplusplus
}
#endif
