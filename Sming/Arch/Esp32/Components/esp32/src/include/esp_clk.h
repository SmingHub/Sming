#pragma once

#include <c_types.h>
#if ESP_IDF_VERSION_MAJOR < 5
#include <hal/cpu_hal.h>
#else
#include <esp_cpu.h>
#endif
#include <sming_attr.h>
#include <esp_idf_version.h>

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
#if ESP_IDF_VERSION_MAJOR < 5
	return cpu_hal_get_cycle_count();
#else
	return esp_cpu_get_cycle_count();
#endif
}

#ifdef __cplusplus
}
#endif
