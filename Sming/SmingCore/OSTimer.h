/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/*
 * 13/8/2018 (mikee47)
 *
 * This class wraps the OS timer functions.
 *
 * For many timing operations the basic OS Timer capabilities are sufficient.
 * The additional RAM required by the main Timer class soon adds up, so all
 * short timer requirements are handled by an OSTimer. Still require Timer for
 * longer periods and the additional callback flexibility.
 *
*/

/** @defgroup   timer OSTimer functions
 *  @brief      Provides timer functions
*/

#ifndef _SMING_CORE_OSTimer_H_
#define _SMING_CORE_OSTimer_H_

#include "esp_systemapi.h"

/*
 * According to documentation maximum value of interval for ms
 * timer after doing system_timer_reinit is 268435ms.
 * If we do some testing we find that higher values works,
 * the actual limit seems to be about twice as high
 * but we use the documented value anyway to be on the safe side.
*/
#define MAX_OS_TIMER_INTERVAL_US 268435000

class OSTimer {
public:
	/** @brief  OSTimer class
     *  @ingroup timer
     *  @{
     */
	OSTimer()
	{}

	~OSTimer()
	{
		stop();
	}

	/** @brief  Initialise millisecond timer
     *  @param  milliseconds Duration of timer in milliseconds
     *  @param  callback Function to call when timer triggers
     *  @note   Classic c-style callback method
     */
	void startMs(uint32_t milliseconds, bool repeating = false)
	{
		stop();
		if (_timer.timer_func)
			ets_timer_arm_new(&_timer, milliseconds, repeating, true);
		else
			debug_e("Timer callback not set");
	}

	/** @brief  Initialise microsecond timer
     *  @param  microseconds Duration of timer in milliseconds
     *  @param  callback Function to call when timer triggers
     *  @note   Classic c-style callback method
     */
	void startUs(uint32_t microseconds, bool repeating = false)
	{
		stop();
		if (_timer.timer_func)
			ets_timer_arm_new(&_timer, microseconds, repeating, false);
		else
			debug_e("Timer callback not set");
	}

	/** @brief  Stop timer
     *  @note   Stops a running timer. Has no effect on stopped timer.
     */
	void stop()
	{
		ets_timer_disarm(&_timer);
	}

	/** @brief  Set timer trigger function
     *  @param  interrupt Function to be called on timer trigger
     *  @note   Classic c-type callback method
     */
	void setCallback(os_timer_func_t callback, void* arg = nullptr)
	{
		stop();
		ets_timer_setfn(&_timer, callback, arg);
	}

private:
	os_timer_t _timer;
};

#endif /* _SMING_CORE_OSTimer_H_ */
