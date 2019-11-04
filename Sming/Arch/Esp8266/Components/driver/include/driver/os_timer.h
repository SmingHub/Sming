/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * os_timer.h
 *
 * @author: 13 August 2018 - mikee47 <mike@sillyhouse.net>
 *
 * An alternative method for setting software timers based on the tick count.
 *
 */

#pragma once

#include <esp_systemapi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set a software timer using the Timer2 tick value
 * @param ptimer Timer structure
 * @param ticks Tick count duration for the timer
 * @param repeat_flag true if timer will automatically repeat
 */
void IRAM_ATTR os_timer_arm_ticks(os_timer_t* ptimer, uint32_t ticks, bool repeat_flag);

#ifdef __cplusplus
}
#endif
