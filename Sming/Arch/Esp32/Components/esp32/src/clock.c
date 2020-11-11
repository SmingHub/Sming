#include "include/esp_clk.h"
#include "include/esp_system.h"

// The current CPU frequency in MHz (ticks per us)
static uint8_t cpu_frequency = SYS_CPU_80MHZ;

bool system_update_cpu_freq(uint8 freq)
{
	if(freq == SYS_CPU_80MHZ || freq == SYS_CPU_160MHZ) {
		cpu_frequency = freq;
		return true;
	} else {
		return false;
	}
}

uint32_t system_get_cpu_freq(void)
{
	return ets_get_cpu_frequency();
}

/*
 * The 'correct' conversion is actually:
 *
 * 		`os_get_nanoseconds() / (1000UL * cpu_frequency)`
 *
 * However, in use this just ends up returning 0 all the time which is
 * not particularly useful.
 *
 * On my dev. system a straight nanosecond count gives quite useful
 * values when evaluating code paths. Try :sample:`Basic_Delegates`.
 *
 */
uint32_t esp_get_ccount()
{
	return os_get_nanoseconds();
}
