// Based on mziwisky espmissingincludes.h && ESP8266_IoT_SDK_Programming Guide_v0.9.1.pdf && ESP SDK defines

#ifndef __ESP_SYSTEM_API_H__
#define __ESP_SYSTEM_API_H__

#include "ets_sys.h"
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <user_interface.h>
#include <spi_flash.h>
#include <espconn.h>
#include "espinc/uart.h"
#include "espinc/uart_register.h"
#include "espinc/spi_register.h"

#include <stdarg.h>

#include <user_config.h>

#include "m_printf.h"
#include "debug_progmem.h"
#include "stringutil.h"

#define __ESP8266_EX__ // System definition ESP8266 SOC

#define IRAM_ATTR __attribute__((section(".iram.text")))
#define __forceinline __attribute__((always_inline)) inline
#define STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define STORE_ATTR __attribute__((aligned(4)))

#ifdef ENABLE_GDB
	#define GDB_IRAM_ATTR IRAM_ATTR
#else
	#define GDB_IRAM_ATTR
#endif

#undef assert
#ifdef SMING_RELEASE
#define debugf(fmt, ...)
#else
#define debugf debug_i
#endif
#define assert(condition) if (!(condition)) SYSTEM_ERROR("ASSERT: %s %d", __FUNCTION__, __LINE__)
#define SYSTEM_ERROR(fmt, ...) m_printf("ERROR: " fmt "\r\n", ##__VA_ARGS__)
// CPU Frequency
extern void ets_update_cpu_frequency(uint32_t frq);
extern uint32_t ets_get_cpu_frequency();

extern void xt_disable_interrupts();
extern void xt_enable_interrupts();

extern void uart_tx_one_char(char ch);
extern void ets_isr_mask(unsigned intr);
extern void ets_isr_unmask(unsigned intr);

typedef signed short file_t;

#endif
