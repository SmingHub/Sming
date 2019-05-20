/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * esp_systemapi.h
 *
 * Based on mziwisky espmissingincludes.h && ESP8266_IoT_SDK_Programming Guide_v0.9.1.pdf && ESP SDK defines
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ESP SDK config
#define LWIP_OPEN_SRC
#define USE_US_TIMER

// Default types
#define __CORRECT_ISO_CPP_STDLIB_H_PROTO
#include <limits.h>
#include "c_types.h"

// Remove buggy espconn
#define _NO_ESPCON_

/* Omitted from early SDK versions, in c_types.h in later versions (note: we don't use the SDK c_types.h) */
typedef enum {
    OK = 0,
    FAIL,
    PENDING,
    BUSY,
    CANCEL,
} STATUS;

#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>

#include "m_printf.h"
#include "debug_progmem.h"
#include "stringutil.h"

#define __ESP8266_EX__ // System definition ESP8266 SOC

#define IRAM_ATTR __attribute__((section(".iram.text")))
#define STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define STORE_ATTR __attribute__((aligned(4)))

/*
 * Use this definition in the cases where a function or a variable is meant to be possibly unused. GCC will not produce a warning for it.
 */
#define SMING_UNUSED  __attribute__((unused))

/*
 * Flags a compiler warning when Sming framework methods, functions or types are changed
 */
#define SMING_DEPRECATED __attribute__((deprecated))

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
#define assert(condition) \
	do {\
		if (!(condition)) SYSTEM_ERROR("ASSERT: %s %d", __FUNCTION__, __LINE__); \
	} while(0)
#define SYSTEM_ERROR(fmt, ...) debug_e("ERROR: " fmt "\r\n", ##__VA_ARGS__)

extern void ets_wdt_enable(void);
extern void ets_wdt_disable(void);
extern void wdt_feed(void);

#ifndef SDK_INTERNAL
extern void ets_timer_arm_new(ETSTimer *ptimer, uint32_t milliseconds, bool repeat_flag, int isMstimer);
extern void ets_timer_disarm(ETSTimer *a);
extern void ets_timer_setfn(ETSTimer *t, ETSTimerFunc *pfunction, void *parg);

extern void ets_delay_us(uint32_t us);

extern void ets_isr_mask(unsigned intr);
extern void ets_isr_unmask(unsigned intr);

typedef void (* ets_isr_t)(void *);

extern void ets_isr_attach(int i, ets_isr_t func, void *arg);

extern int ets_memcmp(const void *s1, const void *s2, size_t n);
extern void *ets_memcpy(void *dest, const void *src, size_t n);
extern void *ets_memset(void *s, int c, size_t n);

extern void ets_install_putc1(void (*p)(char c));
extern int ets_sprintf(char *str, const char *format, ...)  __attribute__ ((format (printf, 2, 3)));
extern int ets_str2macaddr(void *, void *);
extern int ets_strcmp(const char *s1, const char *s2);
extern char *ets_strcpy(char *dest, const char *src);
const char * ets_strrchr(const char *str, int character);
extern int ets_strlen(const char *s);
extern int ets_strncmp(const char *s1, const char *s2, unsigned int len);
extern char *ets_strncpy(char *dest, const char *src, size_t n);
extern char *ets_strstr(const char *haystack, const char *needle);
extern int os_printf_plus(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
extern int os_snprintf(char *str, size_t size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
extern int ets_vsnprintf(char * s, size_t n, const char * format, va_list arg) __attribute__ ((format (printf, 3, 0)));

extern void ets_intr_lock();
extern void ets_intr_unlock();

// Missing from SDK 1.5.x
extern void NmiTimSetFunc(void (*func)(void));

#endif /* SDK_INTERNAL */

// CPU Frequency
extern void ets_update_cpu_frequency(uint32_t frq);
extern uint32_t ets_get_cpu_frequency();

extern void xt_disable_interrupts();
extern void xt_enable_interrupts();

extern void ets_isr_mask(unsigned intr);
extern void ets_isr_unmask(unsigned intr);

#ifdef __cplusplus
}
#endif
