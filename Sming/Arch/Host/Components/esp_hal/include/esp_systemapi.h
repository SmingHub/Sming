/*
 * esp_systemapi.h
 *
 *  Created on: 28 Aug 2018
 *      Author: Mike
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ESP SDK config
#define LWIP_OPEN_SRC
#define USE_US_TIMER

// Default types
#include <limits.h>
#include "c_types.h"

struct ip_addr {
  uint32_t addr;
};

#include <sming_attr.h>
#include "esp_attr.h"
#include "esp_clk.h"
#include "esp_libc.h"
#include "esp_tasks.h"
#include "esp_timer_legacy.h"
#include <heap.h>
#include "esp_system.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include <hostlib/hostapi.h>
#include "esp_sleep.h"

#include <stdarg.h>

#include <m_printf.h>
#include <debug_progmem.h>
#include <stringutil.h>

#include <assert.h>

#include "debug_progmem.h"
#define debugf debug_i

#define SYSTEM_ERROR(fmt, ...) hostmsg("ERROR: " fmt "\r\n", ##__VA_ARGS__)

#define noInterrupts()
#define interrupts()


//#include <string.h>
//#include <assert.h>

#ifdef __cplusplus
}
#endif
