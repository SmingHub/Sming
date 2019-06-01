#pragma once
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void system_restart(void);

void system_soft_wdt_stop(void);
void system_soft_wdt_restart(void);
void system_soft_wdt_feed(void);

/* Arch/Esp8266/Components/spiffs/spiffs_sming.c */
#define ETS_INTR_LOCK() ets_intr_lock()
#define ETS_INTR_UNLOCK() ets_intr_unlock()

void ets_intr_lock();
void ets_intr_unlock();
void xt_disable_interrupts();
void xt_enable_interrupts();

enum rst_reason {
	REASON_DEFAULT_RST = 0,
	REASON_WDT_RST = 1,
	REASON_EXCEPTION_RST = 2,
	REASON_SOFT_WDT_RST = 3,
	REASON_SOFT_RESTART = 4,
	REASON_DEEP_SLEEP_AWAKE = 5,
	REASON_EXT_SYS_RST = 6
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

/* System/include/debug_progmem.h */
uint32_t system_get_time(void);

/* Implementing NOW() macro, fixed at 1us */
#define TIMER_CLK_FREQ 1000000UL
uint32_t os_get_ticks();
#define NOW() os_get_ticks()

void os_delay_us(uint32_t us);

const char* system_get_sdk_version(void);

#ifdef __cplusplus
}
#endif
