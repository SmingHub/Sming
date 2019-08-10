#include "include/esp_system.h"
#include <hostlib/hostapi.h>
#include <hostlib/threads.h>
#include <sys/time.h>

/* System time */

// From pthreads library
extern "C" int sched_yield(void);

#ifdef __WIN32

static struct {
	LARGE_INTEGER cps;
	LARGE_INTEGER startCount;
} timeref;

#endif

// Return base time in us
static uint64_t initTime()
{
#ifdef __WIN32
	QueryPerformanceFrequency(&timeref.cps);
	QueryPerformanceCounter(&timeref.startCount);
#endif

	timeval tv;
	gettimeofday(&tv, nullptr);
	return (1000000ULL * tv.tv_sec) + tv.tv_usec;
}

static uint64_t system_start_time = initTime();

uint32_t usToTimerTicks(uint32_t us)
{
	return us;
}

uint32_t timerTicksToUs(uint32_t ticks)
{
	return ticks;
}

uint32_t os_get_ticks()
{
	uint32_t us;
#ifdef __WIN32
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	us = 1000000ULL * (count.QuadPart - timeref.startCount.QuadPart) / timeref.cps.QuadPart;
#else
	timeval tv;
	gettimeofday(&tv, nullptr);
	us = ((1000000ULL * tv.tv_sec) + tv.tv_usec) - system_start_time;
#endif
	return us;
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

uint32 system_get_chip_id(void)
{
	return 0xC001BEAF;
}

/* Interrupts */

void ets_intr_lock()
{
	CThread::interrupt_lock();
}

void ets_intr_unlock()
{
	CThread::interrupt_unlock();
}

void xt_disable_interrupts()
{
	ets_intr_lock();
}

void xt_enable_interrupts()
{
	ets_intr_unlock();
}
