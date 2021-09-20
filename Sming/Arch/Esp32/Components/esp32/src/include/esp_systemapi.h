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

// ==== taken from ESP8266/ets_sys.h
#define ETS_GPIO_INUM 4

// ====

// Default types
#include <limits.h>
#include <assert.h>

#include <sming_attr.h>
#include <esp_attr.h>
#include "esp_libc.h"
#include "esp_sleep.h"
#include "esp_clk.h"
#include "esp_tasks.h"
#include <heap.h>
#include <esp_system.h>
#include <rom/ets_sys.h>
#include <soc/soc.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

#include <m_printf.h>
#include <debug_progmem.h>
#include <stringutil.h>

#define __ESP32_EX__ // System definition ESP8266 SOC

#define LOCAL static

#define SYSTEM_ERROR(fmt, ...) debug_e("ERROR: " fmt "\r\n", ##__VA_ARGS__)

/** @brief  Disable interrupts
 *  @retval Current interrupt level
 */
#define noInterrupts() portENTER_CRITICAL_NESTED()

/** @brief  Enable interrupts
*/
#define interrupts() portENABLE_INTERRUPTS()

/** @brief Restore interrupts to level saved from previous noInterrupts() call
 */
#define restoreInterrupts(state) portEXIT_CRITICAL_NESTED(state)
