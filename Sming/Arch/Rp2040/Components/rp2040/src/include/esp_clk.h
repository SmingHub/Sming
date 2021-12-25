#pragma once

#include <c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

bool system_update_cpu_freq(uint8_t freq);

uint8_t system_get_cpu_freq(void);

uint32_t esp_get_ccount();

#ifdef __cplusplus
}
#endif
