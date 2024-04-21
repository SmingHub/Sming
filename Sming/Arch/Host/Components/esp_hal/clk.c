#include "include/esp_clk.h"
#include "include/esp_system.h"

// The current CPU frequency in MHz (ticks per us)
static uint8_t cpu_frequency = SYS_CPU_80MHZ;
static uint64_t base_nanos;
static uint32_t base_ccount;

/*
 * Ensure values returned increase monotonically.
 */
static uint32_t get_ccount(uint64_t nanos)
{
	if(base_nanos == 0) {
		base_nanos = nanos;
		base_ccount = nanos / cpu_frequency;
		return base_ccount;
	}

	return base_ccount + cpu_frequency * ((nanos - base_nanos) / 1000);
}

bool system_update_cpu_freq(uint8_t freq)
{
	if(freq == cpu_frequency) {
		return true;
	}

	if(freq != SYS_CPU_80MHZ && freq != SYS_CPU_160MHZ) {
		return false;
	}

	uint64_t nanos = os_get_nanoseconds();
	base_ccount = get_ccount(nanos);
	base_nanos = nanos;
	cpu_frequency = freq;

	return true;
}

uint8_t ets_get_cpu_frequency(void)
{
	return cpu_frequency;
}

uint8_t system_get_cpu_freq(void)
{
	return ets_get_cpu_frequency();
}

uint32_t esp_get_ccount()
{
	return get_ccount(os_get_nanoseconds());
}
