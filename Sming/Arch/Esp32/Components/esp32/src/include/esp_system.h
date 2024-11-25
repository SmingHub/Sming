#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#include_next <esp_system.h>
#include <rom/ets_sys.h>

#if ESP_IDF_VERSION_MAJOR >= 5
#include <esp_random.h>
#endif

#pragma GCC diagnostic pop

uint32_t system_get_time(void);
void system_restart(void);

void system_soft_wdt_stop(void);
void system_soft_wdt_restart(void);
void system_soft_wdt_feed(void);

enum rst_reason {
	REASON_DEFAULT_RST = ESP_RST_UNKNOWN,
	REASON_WDT_RST = ESP_RST_WDT,
	REASON_EXCEPTION_RST = ESP_RST_PANIC,
	REASON_SOFT_WDT_RST = ESP_RST_TASK_WDT,
	REASON_SOFT_RESTART = ESP_RST_SW,
	REASON_DEEP_SLEEP_AWAKE = ESP_RST_DEEPSLEEP,
	REASON_EXT_SYS_RST = ESP_RST_POWERON,
};

struct rst_info {
	uint32_t reason;
	uint32_t exccause;
	uint32_t epc1;
	uint32_t epc2;
	uint32_t epc3;
	uint32_t excvaddr;
	uint32_t depc;
};

struct rst_info* system_get_rst_info(void);

inline void os_delay_us(uint32_t us)
{
	return ets_delay_us(us);
}

const char* system_get_sdk_version(void);

uint32_t system_get_chip_id(void);

unsigned long os_random(void);

int os_get_random(unsigned char* buf, size_t len);

#ifdef __cplusplus
}
#endif
