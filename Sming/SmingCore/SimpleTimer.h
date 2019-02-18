/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SimpleTimer.h
 *
 * @author: 13 August 2018 - mikee47 <mike@sillyhouse.net>
 *
 * This class wraps the OS timer functions, in a class called SimpleTimer.
 *
 * For many timing operations the basic SimpleTimer capabilities are sufficient.
 * The additional RAM required by the main Timer class soon adds up, so all
 * short timer requirements are handled by an SimpleTimer. Still require Timer for
 * longer periods and the additional callback flexibility.
 *
*/

/** @defgroup   timer SimpleTimer functions
 *  @brief      Provides basic OS timer functions
*/

#ifndef _SMING_CORE_SIMPLE_TIMER_H_
#define _SMING_CORE_SIMPLE_TIMER_H_

extern "C" {
#include "esp_systemapi.h"
}

/*
 * According to documentation maximum value of interval for ms
 * timer after doing system_timer_reinit is 268435ms.
 * If we do some testing we find that higher values works,
 * the actual limit seems to be about twice as high
 * but we use the documented value anyway to be on the safe side.
*/
#define MAX_OS_TIMER_INTERVAL_US 268435000

class SimpleTimer
{
public:
	/** @brief  OSTimer class
     *  @ingroup timer
     *  @{
     */
	SimpleTimer()
	{
	}

	~SimpleTimer()
	{
		stop();
	}

	/** @brief  Initialise millisecond timer
     *  @param  milliseconds Duration of timer in milliseconds
     *  @param  callback Function to call when timer triggers
     *  @note   Classic c-style callback method
     */
	__forceinline void startMs(uint32_t milliseconds, bool repeating = false)
	{
		stop();
		if(osTimer.timer_func)
			ets_timer_arm_new(&osTimer, milliseconds, repeating, true);
	}

	/** @brief  Initialise microsecond timer
     *  @param  microseconds Duration of timer in milliseconds
     *  @param  callback Function to call when timer triggers
     *  @note   Classic c-style callback method
     */
	__forceinline void startUs(uint32_t microseconds, bool repeating = false)
	{
		stop();
		if(osTimer.timer_func)
			ets_timer_arm_new(&osTimer, microseconds, repeating, false);
	}

	/** @brief  Stop timer
     *  @note   Stops a running timer. Has no effect on stopped timer.
     */
	__forceinline void stop()
	{
		ets_timer_disarm(&osTimer);
	}

	/** @brief  Set timer trigger function
     *  @param  interrupt Function to be called on timer trigger
     *  @note   Classic c-type callback method
     */
	void setCallback(os_timer_func_t callback, void* arg = nullptr)
	{
		stop();
		ets_timer_setfn(&osTimer, callback, arg);
	}

private:
	os_timer_t osTimer;
};

#endif /* _SMING_CORE_SIMPLE_TIMER_H_ */
