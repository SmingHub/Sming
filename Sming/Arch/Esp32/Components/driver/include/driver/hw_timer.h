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

#include <esp_attr.h>
#include <sming_attr.h>
#include <cstdint>

#define HW_TIMER_BASE_CLK APB_CLK_FREQ

/**
 * @defgroup hw_timer Hardware Timer Driver
 * @ingroup drivers
 * @{
 */

/*************************************
 *
 * Timer1
 * 
 * Used to implement HardwareTimer class.
 *
 *************************************/

// Timer group/index to use: available on all ESP32 variants
#define HW_TIMER1_GROUP TIMER_GROUP_0
#define HW_TIMER1_INDEX TIMER_0

/**
 * @brief Maximum timer interval in ticks
 * @note The corresponding time interval depends on the prescaler in use:
 * 
 * 		/1 - 26.84s
 * 		/16 - 429.50s
 * 		/256 - 6871.95s
 *
 * ESP32 supports a wide range of prescalers and uses 54-bit counter value.
 * Limiting the range 31 bits avoids issues with overflows and moving to 64-bit calculations.
 */
#define MAX_HW_TIMER1_INTERVAL 0x7fffffff

/**
 * @brief Minimum hardware interval in microseconds
 * @note Attempting to use repetitive interrupts below this level can lead
 * to system instabliity and lockups, due to the software overhead in servicing
 * the interrupts.
 */
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

/**
 * @brief Attach an interrupt for the timer
 * @param source_type Ignored, uses APB clock source
 * @param callback Callback function invoked via timer interrupt
 * @param arg Passed to callback function
 */
void IRAM_ATTR hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg);

/**
 * @brief Enable the timer
 * @param div
 * @param intr_type Ignored, always level-triggered
 * @param auto_load
 */
void IRAM_ATTR hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load);

/**
 * @brief Set the timer interval
 * @param ticks
 */
void IRAM_ATTR hw_timer1_write(uint32_t ticks);

/**
 * @brief Disable the timer
 */
void IRAM_ATTR hw_timer1_disable(void);

/**
 * @brief Detach interrupt from the timer
 */
void IRAM_ATTR hw_timer1_detach_interrupt(void);

/**
 * @brief Get timer1 count
 * @retval uint32_t Current count value, counts from initial value down to 0
 */
uint32_t hw_timer1_read(void);

/*************************************
 *
 * Timer2 uses the idf `esp_timer` component for software-based timers (os_timer.cpp).
 *
 *************************************/

constexpr uint32_t HW_TIMER2_CLK = 1000000;

extern "C" int64_t esp_timer_get_time(void);

/**
 * @brief Read current timer2 value
 * @retval uint32_t
 */
__forceinline uint32_t hw_timer2_read(void)
{
	return esp_timer_get_time();
}

#define NOW() hw_timer2_read()

/**
 * @brief Initialise hardware timers
 * @note Called by startup code
 */
void hw_timer_init(void);

/** @} */
