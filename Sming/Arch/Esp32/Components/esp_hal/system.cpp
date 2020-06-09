#include "../include/esp_system.h"
//#include <hostlib/hostapi.h>
//#include <hostlib/threads.h>
#include <sys/time.h>
#include "espinc/timer_register.h"

#include <Platform/Timers.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

/* System time */

static struct {
	uint64_t startTicks;
} timeref;

// Return base time in us
static uint64_t initTime()
{

	timeval tv = {};
	gettimeofday(&tv, nullptr);
	return (1000000ULL * tv.tv_sec) + tv.tv_usec;
}

uint64_t host_system_start_time = initTime();

uint64_t os_get_nanoseconds()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (1000000000ULL * ts.tv_sec) + ts.tv_nsec - timeref.startTicks;
}


// TODO: Remove - defined in $IDF_PATH/components/newlib/time.c
//uint32_t system_get_time()
//{
//	return os_get_nanoseconds() / 1000U;
//}

void os_delay_us(uint32_t us)
{
	vTaskDelay(us / 1000 / portTICK_PERIOD_MS);
}

/* Core system */

struct rst_info* system_get_rst_info(void)
{
	static rst_info info = {REASON_DEFAULT_RST};
	return &info;
}

void system_restart(void)
{
	esp_restart();
}

/* Watchdog */

void system_soft_wdt_feed(void)
{
	esp_task_wdt_reset();
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
	static const char version_string[] = "ESP-IDF v" IDF_VER;
	return version_string;
}

uint32 system_get_chip_id(void)
{
	uint8_t baseMac[6];
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	return (uint32_t)(*(baseMac+2)); // get the last 4 bytes. they should be unique per device
}

/* Interrupts */

void ets_intr_lock()
{
//	CThread::interrupt_lock();
}

void ets_intr_unlock()
{
//	CThread::interrupt_unlock();
}

void xt_disable_interrupts()
{
	ets_intr_lock();
}

void xt_enable_interrupts()
{
	ets_intr_unlock();
}
