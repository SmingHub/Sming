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

#include <esp_systemapi.h>
#include <hardware/timer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HW_TIMER_BASE_CLK 1000000U

#define HW_TIMER_NUM 0
#define HW_TIMER_INST TIMER_INSTANCE(HW_TIMER_NUM)

/**
 * @defgroup hw_timer Hardware Timer Driver
 * @ingroup drivers
 * 
 * @note Timers are 64-bit but for compatibility limit to 31 bits
 * @{
 */

/**
 * @brief Fetch 32-bit microsecond count
 *
 * All timers reference from a single 64-bit counter.
 * We use only the lower 32 bits here as it provides lowest latency
 * and compatibility with existing API.
 */
__forceinline uint32_t IRAM_ATTR hw_timer_ticks()
{
	return timer_time_us_32(HW_TIMER_INST);
}

/*************************************
 *
 * Timer1 (countdown alarm)
 *
 *************************************/

/**
 * @brief Maximum timer interval in ticks
 */
#define MAX_HW_TIMER1_INTERVAL 0x7fffffffU

/**
 * @brief Minimum hardware interval in microseconds
 * @note Attempting to use repetitive interrupts below this level can lead
 * to system instabliity and lockups, due to the software overhead in servicing
 * the interrupts.
 */
#define MIN_HW_TIMER1_INTERVAL_US 50U

typedef void (*hw_timer_callback_t)(void* arg);

typedef enum {
	TIMER_CLKDIV_1,
	TIMER_CLKDIV_16,
	TIMER_CLKDIV_256,
} hw_timer_clkdiv_t;

typedef enum {
	TIMER_EDGE_INT,  // edge interrupt
	TIMER_LEVEL_INT, // level interrupt
} hw_timer_intr_type_t;

typedef enum {
	TIMER_FRC1_SOURCE,
	TIMER_NMI_SOURCE,
} hw_timer_source_type_t;

// Internal data
struct hw_timer_private_t {
	hw_timer_clkdiv_t timer1_clkdiv;
	hw_timer_callback_t timer1_callback;
	uint32_t timer1_ticks;
	bool timer1_autoload;
	void* timer1_arg;
};

extern struct hw_timer_private_t hw_timer_private;

/**
 * @brief Attach an interrupt for the timer
 * @param source_type
 * @param callback Callback function invoked via timer interrupt
 * @param arg Passed to callback function
 */
void hw_timer1_attach_interrupt(hw_timer_source_type_t source_type, hw_timer_callback_t callback, void* arg);

/**
 * @brief Detach interrupt from the timer
 */
void hw_timer1_detach_interrupt();

/**
 * @brief Enable the timer
 * @param div
 * @param intr_type
 * @param auto_load
 */
void hw_timer1_enable(hw_timer_clkdiv_t div, hw_timer_intr_type_t intr_type, bool auto_load);

/**
 * @brief Set the timer interval and arm
 * @param ticks
 */
__forceinline void IRAM_ATTR hw_timer1_write(uint32_t ticks)
{
	ticks <<= hw_timer_private.timer1_clkdiv;
	hw_timer_private.timer1_ticks = ticks;
	HW_TIMER_INST->alarm[0] = hw_timer_ticks() + ticks;
}

/**
 * @brief Disable the timer
 */
__forceinline void IRAM_ATTR hw_timer1_disable()
{
	HW_TIMER_INST->armed = BIT(0);
}

/**
 * @brief Get timer1 count
 * @retval uint32_t Current count value, counts from initial value down to 0
 */
__forceinline uint32_t hw_timer1_read()
{
	int time = hw_timer_ticks() - HW_TIMER_INST->alarm[0];
	return (time > 0) ? (time >> hw_timer_private.timer1_clkdiv) : 0;
}

/*************************************
 *
 * FRC2 timer
 *
 * This is a 32-bit count-up timer.
 * Used for software timers - see os_timer.h
 *
 *************************************/

#define HW_TIMER2_CLK HW_TIMER_BASE_CLK

/**
 * @brief Read current timer2 value
 * @retval uint32_t
 */
__forceinline uint32_t hw_timer2_read()
{
	return hw_timer_ticks();
}

/** @} */

#ifdef __cplusplus
}
#endif
