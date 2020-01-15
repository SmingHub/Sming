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

// Default types
#define __CORRECT_ISO_CPP_STDLIB_H_PROTO
#include <limits.h>
#include "c_types.h"
#include <assert.h>

// Remove buggy espconn
#define _NO_ESPCON_

#include <sming_attr.h>
#include "esp_attr.h"
#include <espinc/pin_mux_register.h>
#include <espinc/gpio_register.h>
#include <espinc/timer_register.h>
#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include "esp_missing.h"
#include <user_interface.h>
#include <espconn.h>

#include "m_printf.h"
#include "debug_progmem.h"
#include "stringutil.h"
#include "xtensa/xtruntime.h"

#define __ESP8266_EX__ // System definition ESP8266 SOC

#define LOCAL           static

#define SYSTEM_ERROR(fmt, ...) debug_e("ERROR: " fmt "\r\n", ##__VA_ARGS__)

extern void ets_wdt_enable(void);
extern void ets_wdt_disable(void);
extern void wdt_feed(void);


/** @brief  Disable interrupts
 *  @retval Current interrupt level
 *  @note Hardware timer is unaffected if operating in non-maskable mode
 */
#define noInterrupts() XTOS_SET_INTLEVEL(15)

/** @brief  Enable interrupts
*/
#define interrupts() XTOS_SET_INTLEVEL(0)

/** @brief Restore interrupts to level saved from previous noInterrupts() call
 */
#define restoreInterrupts(level) XTOS_RESTORE_INTLEVEL(level)

#ifdef __cplusplus
}
#endif
