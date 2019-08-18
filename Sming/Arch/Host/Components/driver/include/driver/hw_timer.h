/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * hw_timer.h
 *
 ****/

#pragma once

#include <stdint.h>
#include <esp_attr.h>
#include <sming_attr.h>

#define APB_CLK_FREQ 80000000U

#define HW_TIMER_BASE_CLK APB_CLK_FREQ

#ifdef __cplusplus
extern "C" {
#endif

/*************************************
 *
 * FRC1 timer
 *
 * This is a 23-bit countdown timer
 *
 *************************************/

#define MAX_HW_TIMER1_INTERVAL 0x7fffff
#define MIN_HW_TIMER1_INTERVAL_US 50U

typedef void (*hw_timer_callback_t)(void* arg);

typedef enum {
	TIMER_CLKDIV_1 = 0,
	TIMER_CLKDIV_16 = 4,
	TIMER_CLKDIV_256 = 8,
} hw_timer_clkdiv_t;

typedef enum {
	TIMER_EDGE_INT = 0,  // edge interrupt
	TIMER_LEVEL_INT = 1, // level interrupt
} hw_timer_intr_type_t;

typedef enum {
	TIMER_FRC1_SOURCE = 0,
	TIMER_NMI_SOURCE = 1,
} hw_timer_source_type_t;

void hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg);
void hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load);
void hw_timer1_write(uint32_t ticks);
void hw_timer1_disable(void);
void hw_timer1_detach_interrupt(void);
uint32_t hw_timer1_read(void);

/*************************************
 *
 * FRC2 timer
 *
 * This is a 32-bit count-up timer
 *
 *************************************/

#ifdef USE_US_TIMER
constexpr uint32_t HW_TIMER2_CLK = HW_TIMER_BASE_CLK / 16;
#else
constexpr uint32_t HW_TIMER2_CLK = HW_TIMER_BASE_CLK / 256;
#endif

uint32_t hw_timer2_read(void);

inline uint32_t NOW()
{
	return hw_timer2_read();
}

inline void hw_timer_init(void)
{
}

#ifdef __cplusplus
}
#endif
