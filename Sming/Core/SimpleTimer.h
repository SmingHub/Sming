/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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

/** @ingroup   	timer
 *  @brief      Provides basic OS timer functions
*/

#pragma once

#include "esp_systemapi.h"

/*
 * According to documentation maximum value of interval for ms
 * timer after doing system_timer_reinit is 268435ms.
 * If we do some testing we find that higher values works,
 * the actual limit seems to be about twice as high
 * but we use the documented value anyway to be on the safe side.
*/
#define MAX_OS_TIMER_INTERVAL_US 268435000

typedef os_timer_func_t* SimpleTimerCallback;

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
     *  @param  repeating true if timer automatically restarts when it expires
     */
	__forceinline void startMs(uint32_t milliseconds, bool repeating = false)
	{
		stop();
		if(osTimer.timer_func)
			os_timer_arm(&osTimer, milliseconds, repeating);
	}

	/** @brief  Initialise microsecond timer
     *  @param  microseconds Duration of timer in milliseconds
     *  @param  repeating true if timer automatically restarts when it expires
     */
	__forceinline void startUs(uint32_t microseconds, bool repeating = false)
	{
		stop();
		if(osTimer.timer_func)
			os_timer_arm_us(&osTimer, microseconds, repeating);
	}

	/** @brief  Stop timer
     *  @note   Stops a running timer. Has no effect on stopped timer.
     */
	__forceinline void stop()
	{
		os_timer_disarm(&osTimer);
	}

	/** @brief  Set timer trigger function
     *  @param  callback Function to be called on timer trigger
     *  @param	arg Passed to callback
     *  @note   Classic c-type callback method
     */
	void setCallback(SimpleTimerCallback callback, void* arg = nullptr)
	{
		stop();
		os_timer_setfn(&osTimer, callback, arg);
	}

private:
	os_timer_t osTimer;
};
