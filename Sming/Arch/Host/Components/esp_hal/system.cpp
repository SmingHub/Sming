#include "include/esp_system.h"
#include <hostlib/hostapi.h>

/* System time */

#include <chrono>
using namespace std::chrono;

static high_resolution_clock::time_point system_start_time = high_resolution_clock::now();

// From pthreads library
extern "C" int sched_yield(void);

uint32_t os_get_ticks()
{
	return duration_cast<microseconds>(high_resolution_clock::now() - system_start_time).count();
}

uint32_t system_get_time()
{
	return os_get_ticks();
}

void os_delay_us(uint32_t us)
{
	auto start = system_get_time();
	while(system_get_time() - start < us) {
		//
	}
}

/* Core system */

struct rst_info* system_get_rst_info(void)
{
	static rst_info info = {REASON_DEFAULT_RST};
	return &info;
}

void system_restart(void)
{
	host_exit(0);
}

/* Watchdog */

void system_soft_wdt_feed(void)
{
	sched_yield();
}

void system_soft_wdt_stop(void)
{
}

void system_soft_wdt_restart(void)
{
}

/* Misc */

const char* system_get_sdk_version(void)
{
	static const char version_string[] = "Host-SDK-v1.0";
	return version_string;
}

/* Interrupts */

void ets_intr_lock()
{
}

void ets_intr_unlock()
{
}

void xt_disable_interrupts()
{
}

void xt_enable_interrupts()
{
}
