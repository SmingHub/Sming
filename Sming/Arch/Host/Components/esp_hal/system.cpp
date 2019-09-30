#include "include/esp_system.h"
#include <hostlib/hostapi.h>
#include <hostlib/threads.h>
#include <sys/time.h>
#include <Platform/Timers.h>

/* System time */

static struct {
#ifdef __WIN32
	Ratio64 countsPerNanosecond;
	LARGE_INTEGER startCount;
#else
	uint64_t startTicks;
#endif
} timeref;

// Return base time in us
static uint64_t initTime()
{
#ifdef __WIN32
	LARGE_INTEGER cps;
	QueryPerformanceFrequency(&cps);
	timeref.countsPerNanosecond.set(1000000000ULL, cps.QuadPart);
	QueryPerformanceCounter(&timeref.startCount);
#else
	timeref.startTicks = os_get_nanoseconds();
#endif

	timeval tv;
	gettimeofday(&tv, nullptr);
	return (1000000ULL * tv.tv_sec) + tv.tv_usec;
}

uint64_t host_system_start_time = initTime();

uint64_t os_get_nanoseconds()
{
#ifdef __WIN32
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return timeref.countsPerNanosecond * uint64_t(count.QuadPart - timeref.startCount.QuadPart);
#else
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (1000000000ULL * ts.tv_sec) + ts.tv_nsec - timeref.startTicks;
#endif
}

uint32_t system_get_time()
{
	return os_get_nanoseconds() / 1000U;
}

void os_delay_us(uint32_t us)
{
	ElapseTimer timer(us);
	while(!timer.expired()) {
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
