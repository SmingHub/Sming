/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * esp_systemapi.h
 *
 ****/

#pragma once

// Default types
#include <limits.h>
#include <assert.h>

#include <sming_attr.h>
#include <esp_attr.h>
#include <esp_tasks.h>
#include <esp_clk.h>
#include <esp_libc.h>
#include <esp_sleep.h>
#include "esp_system.h"
#include <heap.h>

#include "m_printf.h"
#include "debug_progmem.h"
#include "stringutil.h"

#define BIT(nr) (1UL << (nr))

#define SYSTEM_ERROR(fmt, ...) debug_e("ERROR: " fmt "\r\n", ##__VA_ARGS__)

extern void ets_wdt_enable(void);
extern void ets_wdt_disable(void);
extern void wdt_feed(void);


/** @brief  Disable interrupts
 *  @retval Current interrupt level
 *  @note Hardware timer is unaffected if operating in non-maskable mode
 */
uint32_t noInterrupts();

/** @brief  Enable interrupts
*/
void interrupts();

/** @brief Restore interrupts to level saved from previous noInterrupts() call
 */
void restoreInterrupts(uint32_t level);
