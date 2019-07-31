/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ElapseTimer.h - Class to measure elapsed time periods by polling hardware timer.
 *
 ****/
#pragma once

#include "HardwareTimer.h"

/**
 * @brief Microsecond elapse timer to efficiently measure intervals
 * @note Maintains time using the hardware 'tick' counter values and only performs
 * conversion to microseconds when necessary.
 * The class keeps a note of a 'start' time used to determine the `elapsed()` return value.
 * An 'interval' value may be given is specified, then the `expired()` method returns true.
 */
class ElapseTimer
{
public:
	/**
	 * @brief Create new ElapseTimer with optional expiry time
	 * @param interval Microseconds to expiry after last call to start()
	 * @note Conversion between microseconds and ticks is relatively expensive, so pass 0 if not required.
	 */
	__forceinline ElapseTimer(uint32_t interval = 0)
	{
		setInterval(interval);
		start();
	}

	/**
	 * @brief Set expiry interval
	 * @param interval Microseconds to expiry after last call to start()
	 */
	void __forceinline setInterval(uint32_t interval)
	{
		this->interval = (interval == 0) ? 0 : usToTimerTicks(interval);
	}

	/**
	 * @brief Get the current tick value
	 */
	uint32_t __forceinline ticks()
	{
		return NOW();
	}

	/**
	 * @brief re-start the timer
	 */
	void __forceinline start()
	{
		startTicks = ticks();
	}

	/**
	 * @brief Get elapsed time in microseconds since start() was last called
	 */
	uint32_t __forceinline elapsed()
	{
		return timerTicksToUs(ticks() - startTicks);
	}

	/**
	 * @brief Determine if timer has expired
	 * @retval bool
	 */
	bool __forceinline expired()
	{
		return (ticks() - startTicks) >= interval;
	}

private:
	uint32_t startTicks;
	uint32_t interval;
};
