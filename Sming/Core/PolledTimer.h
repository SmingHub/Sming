/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * PolledTimer.h - template class to assist with measurement of elapsed time periods
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 * Developed from the excellent esp8266 Arduino project's PolledTimer.h
 * 	Copyright (c) 2018 Daniel Salazar. All rights reserved.
 * 	https://github.com/esp8266/Arduino/blob/master/cores/esp8266/PolledTimeout.h
 *
 * Generally Sming uses timer callbacks but for some applications a polled timer
 * is more appropriate, especially if timer intervals are less than few hundred
 * microseconds, or even in nanoseconds.
 *
 * Here we have the `PolledTimer` template class. See `Platform/Timers.h` for implementations.
 *
 ****/

#pragma once

#include <cstdint>
#include <esp_attr.h>
#include <sming_attr.h>
#include <Platform/Clocks.h>

/** @defgroup polled_timer Polled timers
 *  @brief    Polled interval timers
 *  @ingroup  timers
 *  @{
*/

/**
 * @brief Timer intervals are limited to the maximum clock time, minus this safety margin
 * @note Specified in microseconds, this is the minimum timer poll interval to ensure no missed
 * polls across the full timer range. Larger margin means smaller time range.
 */
#ifndef POLLED_TIMER_MARGIN_US
#define POLLED_TIMER_MARGIN_US 250000
#endif

namespace PolledTimer
{
/**
 * @brief Template class to implement a polled timer
 * @tparam Clock Clock source
 * @tparam unit Time unit for tick/time conversions
 * @tparam IsPeriodic
 * @tparam TimeType Variable type to use (uint32_t or uint64_t)
 * @note Intervals and expiry use 'tick' values which are very efficient, whereas 'time' values
 * must be computed so are very slow (~100+ cycles for uint32_t, considerably more for uint64_t).
 * The class keeps a note of a 'start' time (in ticks) used to determine the `elapsed()` return value.
 * An 'interval' value may be given is specified, then the `expired()` method returns true.
 *
 * If the interval is set to 0, the timer will expire immediately, and remain so: `canWait()` will return false.
 * Call `cancel()` to prevent the timer from ever expiring: `canExpire()` will return false.
 * Call `start()` to set the timer going with the previously set interval.
 * Call `reset()` to set the timer with a new interval.
 *
 * A periodic timer will automatically restart when expired.
 * A one-shot timer will remain expired until reset.
 */
template <class Clock, NanoTime::Unit unit_, bool IsPeriodic, typename TimeType>
class Timer : public NanoTime::TimeSource<Clock, unit_, TimeType>
{
public:
	static_assert(std::is_unsigned<TimeType>::value == true, "TimeType must be unsigned");

	using TickType = typename Clock::TickType;
	using Clock::ticks;
	using Margin = NanoTime::TimeConst<Clock, NanoTime::Microseconds, POLLED_TIMER_MARGIN_US>;

	static constexpr NanoTime::Unit unit()
	{
		return unit_;
	}

	static constexpr TickType maxInterval()
	{
		return Clock::maxTicks() - Margin::ticks();
	}

	static constexpr Margin margin()
	{
		return Margin();
	}

	/**
	 * @brief Create a Timer with optional expiry time
	 * @param timeInterval Relative time until expiry
	 */
	IRAM_ATTR Timer(const TimeType& timeInterval = 0)
	{
		reset(timeInterval);
	}

	/**
	 * @brief Start the timer
	 */
	__forceinline void IRAM_ATTR start()
	{
		startTicks = ticks();
		if(!IsPeriodic) {
			// One-shot timers require manual reset
			hasExpired = false;
		}
	}

	/**
	 * @brief Start the timer with a new expiry interval
	 * @tparam timeInterval Time to expire after last call to start()
	 */
	template <uint64_t timeInterval> __forceinline void IRAM_ATTR reset()
	{
		auto ticks = checkTime<timeInterval>();
		resetTicks(ticks);
	}

	/**
	 * @brief Check the given time interval is valid and return the corresponding tick count
	 * @tparam timeInterval
	 * @retval uint32_t
	 * @note If time interval is invalid fails compilation
	 */
	template <uint64_t timeInterval> constexpr uint32_t checkTime()
	{
		auto time = this->template timeConst<timeInterval>();
		time.check();
		constexpr auto ticks = time.ticks();
		static_assert(ticks < maxInterval(), "Polled time interval too long");
		return ticks;
	}

	/**
	 * @brief Start the timer with a new expiry interval
	 * @param timeInterval Time to expire after last call to start()
	 * @retval bool true on success, false on failure
	 * @see See `resetTicks()`
	 */
	__forceinline bool IRAM_ATTR reset(const TimeType& timeInterval)
	{
		return resetTicks(this->template timeToTicks(timeInterval));
	}

	/**
	 * @brief Start the timer with a new expiry interval
	 * @param interval Clock ticks to expire after last call to start()
	 * @retval bool true on success, false if interval is out of range
	 * @note If time interval is 0, timer will expire immediately, and if it
	 * exceeds the maximum interval the timer will never expire.
	 */
	__forceinline bool IRAM_ATTR resetTicks(const TimeType& interval)
	{
		start();
		this->interval = interval;
		neverExpires = (interval > maxInterval());
		return !neverExpires;
	}

	/**
	 * @brief Cancelling a timer means it will never expire
	 */
	__forceinline void IRAM_ATTR cancel()
	{
		interval = 1; // Ensure canWait() returns true
		neverExpires = true;
		if(!IsPeriodic) {
			hasExpired = true;
		}
	}

	/**
	 * @brief Get elapsed ticks since start() was last called
	 */
	__forceinline TickType elapsedTicks() const
	{
		return ticks() - startTicks;
	}

	/**
	 * @brief Get elapsed time since start() was last called
	 */
	__forceinline NanoTime::Time<TimeType> elapsedTime() const
	{
		return this->template ticksToTime(elapsedTicks());
	}

	__forceinline bool canExpire() const
	{
		return !neverExpires;
	}

	__forceinline bool canWait() const
	{
		return interval != 0;
	}

	/**
	 * @brief Determine if timer has expired
	 * @retval bool
	 */
	__forceinline bool expired()
	{
		return IsPeriodic ? expiredRetrigger() : expiredOneShot();
	}

private:
	bool IRAM_ATTR checkExpired(const TickType& ticks) const
	{
		// canWait() is not checked here
		// returns "can expire" and "time expired"
		return !neverExpires && (TickType(ticks - startTicks) >= interval);
	}

	bool IRAM_ATTR expiredRetrigger()
	{
		if(!canWait()) {
			return true;
		}

		bool result = false;
		TickType current = ticks();
		while(checkExpired(current)) {
			result = true;
			startTicks += interval;
		}

		return result;
	}

	bool IRAM_ATTR expiredOneShot()
	{
		// Remain triggered until manually reset or cancelled
		if(!canWait() || hasExpired) {
			return true;
		}
		hasExpired = checkExpired(ticks());
		return hasExpired;
	}

	TickType startTicks;
	TickType interval;
	bool neverExpires; ///< Set when timer cancelled or set to maximal interval
	bool hasExpired;   ///< Set when one-shot timer expiry reported
};

// Standard timer types
template <typename Clock, NanoTime::Unit unit> using OneShot = Timer<Clock, unit, false, uint32_t>;
template <typename Clock, NanoTime::Unit unit> using Periodic = Timer<Clock, unit, true, uint32_t>;

} // namespace PolledTimer

/** @} */
