#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t system_get_time(void);
void system_restart(void);

void system_soft_wdt_stop(void);
void system_soft_wdt_restart(void);
void system_soft_wdt_feed(void);

enum rst_reason {
	REASON_DEFAULT_RST,
	REASON_WDT_RST,
	REASON_EXCEPTION_RST,
	REASON_SOFT_WDT_RST,
	REASON_SOFT_RESTART,
	REASON_DEEP_SLEEP_AWAKE,
	REASON_EXT_SYS_RST,
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

void os_delay_us(uint32_t us);

const char* system_get_sdk_version(void);

uint32_t system_get_chip_id(void);

#ifdef __cplusplus
}
#endif
