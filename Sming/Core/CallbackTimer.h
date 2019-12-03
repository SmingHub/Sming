/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CallbackTimer.h - Template classes to implement callback timers
 *
 ****/

#pragma once

#include "Interrupts.h"
#include "NanoTime.h"

/** @defgroup callback_timer Callback timers
 *  @brief    Callback interval timers
 *  @ingroup  timer
 *  @{
*/

typedef void (*TimerCallback)(void* arg); ///< Interrupt-compatible C callback function pointer
using TimerDelegate = Delegate<void()>;   ///< Delegate callback

/**
 * @brief Callback timer API class template
 * @note Used to define the basic programming interface for physical callback-capable timers
 */
template <typename ApiDef> struct CallbackTimerApi {
	static constexpr const char* typeName()
	{
		return ApiDef::typeName();
	}

	String name() const
	{
		String s;
		s += typeName();
		s += '@';
		s += String(uint32_t(this), HEX);
		return s;
	}

	String toString() const
	{
		String s;
		s += name();
		s += ": interval = ";
		s += static_cast<const ApiDef*>(this)->getInterval();
		s += ", ticks = ";
		s += static_cast<const ApiDef*>(this)->ticks();
		//		s += ApiDef::Clock::ticks();
		return s;
	}

	operator String() const
	{
		return toString();
	}
};

/**
 * @brief Callback timer class template
 * @tparam TimerApi The physical timer implementation
 * @note Methods return object reference for Method Chaining
 * http://en.wikipedia.org/wiki/Method_chaining
 * This class template provides basic C-style callbacks for best performance
 */
template <typename TimerApi> class CallbackTimer : protected TimerApi
{
public:
	using typename TimerApi::Clock;
	using typename TimerApi::TickType;
	using typename TimerApi::TimeType;
	using Millis = NanoTime::TimeSource<Clock, NanoTime::Milliseconds, uint32_t>;
	using Micros = NanoTime::TimeSource<Clock, NanoTime::Microseconds, TimeType>;

	using TimerApi::maxTicks;
	using TimerApi::minTicks;
	using TimerApi::toString;
	using TimerApi::typeName;
	using TimerApi::operator String;

	/** @brief Get a millisecond time source */
	static constexpr Millis millis()
	{
		return Millis();
	}

	/** @brief Get a microsecond time source */
	static constexpr Micros micros()
	{
		return Micros();
	}

	/** @brief Convert microsecond count into timer ticks */
	template <uint64_t us> static constexpr uint64_t usToTicks()
	{
		return Micros::template timeToTicks<us>();
	}

	/** @brief Convert microsecond count into timer ticks */
	static TickType usToTicks(TimeType time)
	{
		return Micros::timeToTicks(time);
	}

	/** @brief Convert timer ticks into microseconds */
	template <uint64_t ticks> static constexpr uint64_t ticksToUs()
	{
		return Micros::template ticksToTime<ticks>();
	}

	/** @brief Convert timer ticks into microseconds */
	static TimeType ticksToUs(TickType ticks)
	{
		return Micros::ticksToTime(ticks);
	}

	/** @brief  Initialise timer with an interval (static check) and callback
	 *  @param  unit Time unit for interval
     *  @param  time Timer interval
     *  @param  callback Callback function to call when timer triggers
     *  @param  arg Optional argument passed to callback
     *  @retval CallbackTimer& Reference to timer
     *  @note   If interval out of range compilation will fail with error
     */
	template <NanoTime::Unit unit, TimeType time>
	CallbackTimer& IRAM_ATTR initialize(TimerCallback callback, void* arg = nullptr)
	{
		setCallback(callback, arg);
		setInterval<unit, time>();
		return *this;
	}

	/** @brief  Initialise timer with an interval and callback
     *  @param  time Timer interval
     *  @param  callback Callback function to call when timer triggers
     *  @param  arg Optional argument passed to callback
     *  @retval CallbackTimer& Reference to timer
     */
	template <NanoTime::Unit unit>
	CallbackTimer& IRAM_ATTR initialize(TimeType time, TimerCallback callback, void* arg = nullptr)
	{
		setCallback(callback, arg);
		setInterval<unit>(time);
		return *this;
	}

	/** @brief  Initialise timer in microseconds (static check) with Timer Callback and optional argument */
	template <TimeType microseconds>
	__forceinline CallbackTimer& IRAM_ATTR initializeUs(TimerCallback callback, void* arg = nullptr)
	{
		return initialize<NanoTime::Microseconds, microseconds>(callback, arg);
	}

	/** @brief  Initialise timer in microseconds (static check) with optional Interrupt Callback (no argument) */
	template <TimeType microseconds>
	__forceinline CallbackTimer& IRAM_ATTR initializeUs(InterruptCallback callback = nullptr)
	{
		return initializeUs<microseconds>(TimerCallback(callback));
	}

	/** @brief  Initialise timer in microseconds with Timer Callback and optional argument */
	__forceinline CallbackTimer& IRAM_ATTR initializeUs(TimeType microseconds, TimerCallback callback,
														void* arg = nullptr)
	{
		return initialize<NanoTime::Microseconds>(microseconds, callback, arg);
	}

	/** @brief  Initialise timer in microseconds with optional Interrupt Callback (no arg) */
	__forceinline CallbackTimer& IRAM_ATTR initializeUs(TimeType microseconds, InterruptCallback callback = nullptr)
	{
		return initializeUs(microseconds, TimerCallback(callback));
	}

	/** @brief  Initialise hardware timer in milliseconds (static check) with Timer Callback and optional argument */
	template <uint32_t milliseconds>
	__forceinline CallbackTimer& IRAM_ATTR initializeMs(TimerCallback callback, void* arg = nullptr)
	{
		return initialize<NanoTime::Milliseconds, milliseconds>(callback, arg);
	}

	/** @brief  Initialise hardware timer in milliseconds (static check) and optional Interrupt Callback (no arg) */
	template <uint32_t milliseconds>
	__forceinline CallbackTimer& IRAM_ATTR initializeMs(InterruptCallback callback = nullptr)
	{
		return initializeMs<milliseconds>(TimerCallback(callback));
	}

	/** @brief  Initialise hardware timer in milliseconds with Timer Callback and optional argument */
	__forceinline CallbackTimer& IRAM_ATTR initializeMs(uint32_t milliseconds, TimerCallback callback,
														void* arg = nullptr)
	{
		return initialize<NanoTime::Milliseconds>(milliseconds, callback, arg);
	}

	/** @brief  Initialise hardware timer in milliseconds with optional Interrupt Callback (no arg) */
	__forceinline CallbackTimer& IRAM_ATTR initializeMs(uint32_t milliseconds, InterruptCallback callback = nullptr)
	{
		return initializeMs(milliseconds, TimerCallback(callback));
	}

	/** @brief  Start timer running
     *  @param  repeating True to restart timer when it triggers, false for one-shot (Default: true)
     *  @retval bool True if timer started
     */
	IRAM_ATTR bool start(bool repeating = true);

	/** @brief  Start one-shot timer
	 *  @retval bool True if timer started
	 *  @note   Timer starts and will run for configured period then stop
	 */
	__forceinline bool IRAM_ATTR startOnce()
	{
		return start(false);
	}

	/** @brief  Stops timer
	 */
	__forceinline void IRAM_ATTR stop()
	{
		if(started) {
			TimerApi::disarm();
			started = false;
		}
	}

	/** @brief  Restart timer
	 *  @retval bool True if timer started
	 *  @note   Timer is stopped then started with current configuration
	 */
	__forceinline bool IRAM_ATTR restart()
	{
		return start(repeating);
	}

	/** @brief  Check if timer is started
	 *  @retval bool True if started
	 */
	__forceinline bool isStarted() const
	{
		return started;
	}

	/** @brief  Get timer interval in microseconds */
	NanoTime::Time<TimeType> getIntervalUs() const
	{
		return Micros::ticksToTime(getInterval());
	}

	/** @brief  Get timer interval in milliseconds */
	NanoTime::Time<uint32_t> getIntervalMs() const
	{
		return Millis::ticksToTime(getInterval());
	}

	/** @brief Get timer interval in clock ticks */
	__forceinline TickType getInterval() const
	{
		return TimerApi::getInterval();
	}

	/** @brief  Check timer interval is valid
     *  @param  ticks Interval to check
     *  @retval bool true if interval is within acceptable range for this timer
     */
	bool IRAM_ATTR checkInterval(TickType ticks) const
	{
		return ticks >= minTicks() && ticks <= maxTicks();
	}

	/** @brief  Check timer interval in ticks is valid (static check)
     *  @tparam ticks Timer interval to check
     *  @note   On error, compilation fails with error message
     */
	template <uint64_t ticks> static constexpr void checkInterval()
	{
		static_assert(ticks >= minTicks() && ticks <= maxTicks(), "Timer interval out of range");
	}

	/** @brief  Check timer interval in specific time unit is valid (static check)
	 *  @tparam unit Time unit for interval
     *  @tparam time Interval to check
     *  @note   On error, compilation fails with error message
     */
	template <NanoTime::Unit unit, uint64_t time> static constexpr void checkInterval()
	{
		checkInterval<Clock::template TimeConst<unit, time>::ticks()>();
	}

	/** @brief Check timer interval in milliseconds is valid (static check) */
	template <uint64_t milliseconds> static constexpr void checkIntervalMs()
	{
		checkInterval<NanoTime::Milliseconds, milliseconds>();
	}

	/** @brief Check timer interval in microseconds is valid (static check) */
	template <uint64_t microseconds> static constexpr void checkIntervalUs()
	{
		checkInterval<NanoTime::Microseconds, microseconds>();
	}

	/** @brief  Set timer interval in timer ticks
     *  @param  ticks Interval in timer ticks
     */
	__forceinline bool IRAM_ATTR setInterval(TickType ticks)
	{
		if(checkInterval(ticks)) {
			internalSetInterval(ticks);
		} else {
			stop();
			intervalSet = false;
		}
		return started;
	}

	/** @brief  Set timer interval in timer ticks (static check)
     *  @tparam ticks Interval in ticks
     *  @note   On error, compilation fails with error message
     */
	template <TimeType ticks> __forceinline void IRAM_ATTR setInterval()
	{
		checkInterval<ticks>();
		internalSetInterval(ticks);
	}

	/** @brief  Set timer interval in specific time unit (static check)
	 *  @tparam unit
     *  @tparam time Interval to set
     *  @note   On error, compilation fails with error message
     */
	template <NanoTime::Unit unit, TimeType time> __forceinline void IRAM_ATTR setInterval()
	{
		setInterval<Clock::template TimeConst<unit, time>::ticks()>();
	}

	/** @brief  Set timer interval in timer ticks
     *  @param  ticks Interval in timer ticks
     */
	template <NanoTime::Unit unit> __forceinline bool IRAM_ATTR setInterval(TimeType time)
	{
		return setInterval(Clock::template timeToTicks<unit>(time));
	}

	/** @brief  Set timer interval in microseconds */
	__forceinline bool IRAM_ATTR setIntervalUs(TimeType microseconds)
	{
		return setInterval<NanoTime::Microseconds>(microseconds);
	}

	/** @brief  Set timer interval in microseconds (static check) */
	template <TimeType microseconds> __forceinline void IRAM_ATTR setIntervalUs()
	{
		return setInterval<NanoTime::Microseconds, microseconds>();
	}

	/** @brief Set timer interval in milliseconds */
	__forceinline bool IRAM_ATTR setIntervalMs(uint32_t milliseconds)
	{
		return setInterval<NanoTime::Milliseconds>(milliseconds);
	}

	/** @brief  Set timer interval in milliseconds (static check) */
	template <uint32_t milliseconds> __forceinline void IRAM_ATTR setIntervalMs()
	{
		return setInterval<NanoTime::Milliseconds, milliseconds>();
	}

	/** @brief Set timer trigger callback
     *  @param callback Function to call when timer triggers
     *  @param  arg Optional argument passed to callback
     */
	__forceinline void IRAM_ATTR setCallback(TimerCallback callback, void* arg = nullptr)
	{
		// Always disarm before setting the callback
		stop();
		TimerApi::setCallback(callback, arg);
		callbackSet = (callback != nullptr);
	}

	/**
	 * @brief Set timer trigger callback
	 * @param callback Function to call when timer triggers
	 * @note  Provided for convenience where callback argument not required
	 */
	__forceinline void IRAM_ATTR setCallback(InterruptCallback callback)
	{
		setCallback(reinterpret_cast<TimerCallback>(callback), nullptr);
	}

private:
	__forceinline void IRAM_ATTR internalSetInterval(TickType ticks)
	{
		TimerApi::disarm();
		TimerApi::setInterval(ticks);
		intervalSet = true;
		if(started) {
			TimerApi::arm(repeating);
		}
	}

protected:
	bool callbackSet = false; ///< User has provided callback function
	bool intervalSet = false; ///< User has set valid time interval
	bool repeating = false;   ///< Timer is auto-repeat
	bool started = false;	 ///< Timer is active, or has fired
};

template <typename TimerApi> IRAM_ATTR bool CallbackTimer<TimerApi>::start(bool repeating)
{
	stop();
	if(!callbackSet || !intervalSet) {
		return false;
	}

	TimerApi::arm(repeating);
	started = true;
	this->repeating = repeating;
	return true;
}

/** @} */
