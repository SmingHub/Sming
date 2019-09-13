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
*/

#pragma once

#include "esp_systemapi.h"
#include "CallbackTimer.h"
#include <Platform/Clocks.h>
#include <driver/os_timer.h>

/**	@ingroup callback_timer
 *  @{
 */

/**
 * @brief Implements common system callback timer API
 */
class OsTimerApi : public CallbackTimerApi<OsTimerApi>
{
public:
	using Clock = Timer2Clock;
	using TickType = uint32_t;
	using TimeType = uint32_t;

	static constexpr const char* typeName()
	{
		return "OsTimerApi";
	}

	static constexpr TickType minTicks()
	{
		// Note: The minimum specified by the SDK is 100us
		return 1;
	}

	static constexpr TickType maxTicks()
	{
		return 0x7FFFFFFF;
	}

	__forceinline bool isArmed() const
	{
		return int(osTimer.timer_next) != -1;
	}

	TickType ticks() const
	{
		if(!isArmed()) {
			return 0;
		}

		auto remain = int(osTimer.timer_expire - Clock::ticks());
		return (remain > 0) ? remain : 0;
	}

	~OsTimerApi()
	{
		disarm();
	}

	__forceinline void IRAM_ATTR setCallback(TimerCallback callback, void* arg)
	{
		os_timer_setfn(&osTimer, callback, arg);
	}

	__forceinline void IRAM_ATTR setInterval(TickType interval)
	{
		this->interval = interval;
	}

	__forceinline TickType IRAM_ATTR getInterval() const
	{
		return interval;
	}

	__forceinline void IRAM_ATTR arm(bool repeating)
	{
		os_timer_arm_ticks(&osTimer, interval, repeating);
	}

	__forceinline void IRAM_ATTR disarm()
	{
		if(isArmed()) {
			os_timer_disarm(&osTimer);
		}
	}

private:
	os_timer_t osTimer = {
		reinterpret_cast<os_timer_t*>(-1), // Disarmed
	};
	TickType interval = 0;
};

/**
 * @brief Basic callback timer
 * @note For delegate callback support and other features see `Timer` class.
 */
using SimpleTimer = CallbackTimer<OsTimerApi>;

/** @} */
