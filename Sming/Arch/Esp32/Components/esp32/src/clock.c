#include <esp_clk.h>
#include <esp_system.h>
#include <esp_attr.h>
#include <ets_sys.h>

// The current CPU frequency in MHz (ticks per us)
static uint8_t cpu_frequency = SYS_CPU_80MHZ;

bool system_update_cpu_freq(uint8_t freq)
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

uint32_t IRAM_ATTR esp_get_ccount()
{
	uint32_t ccount;
	__asm__ __volatile__("esync; rsr %0,ccount" : "=a"(ccount));
	return ccount;
}
