#include "include/esp_clk.h"

// The current CPU frequency
static uint8_t __cpu_frequency = 80;

bool system_update_cpu_freq(uint8 freq)
{
	if(freq == 80 || freq == 160) {
		__cpu_frequency = freq;
		return true;
	} else {
		return false;
	}
}

uint8 system_get_cpu_freq(void)
{
	return __cpu_frequency;
}
