/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Timer.h
 *
 ****/

#pragma once

#include "Interrupts.h"
#include "SimpleTimer.h"

/**
 * @defgroup timer Timer
 * @brief Extended timer queue class
 * @ingroup timers
 * @{
 */

/** @deprecated Use `TimerDelegate` */
typedef std::function<void()> TimerDelegateStdFunction SMING_DEPRECATED;

/**
 * @brief Class template implementing an extended OS Timer with 64-bit microsecond times and delegate callback support
 */
template <class TimerClass> class OsTimer64Api : public CallbackTimerApi<OsTimer64Api<TimerClass>>
{
public:
	using Clock = OsTimerApi::Clock;
	using TickType = uint64_t;
	using TimeType = uint64_t;

	static constexpr const char* typeName()
	{
		return "OsTimer64Api";
	}

	static constexpr TickType minTicks()
	{
		return 1;
	}

	static constexpr TickType maxTicks()
	{
		return Clock::maxTicks() * 0xFFFF;
	}

	OsTimer64Api()
	{
		osTimer.setCallback(
			[](void* arg) {
				auto self = static_cast<OsTimer64Api*>(arg);
				self->longTick();
			},
			this);
	}

	~OsTimer64Api()
	{
		disarm();
	}

	__forceinline bool IRAM_ATTR isArmed() const
	{
		return osTimer.isArmed();
	}

	TickType ticks() const
	{
		TickType remain = osTimer.ticks();
		if(longIntervalCounterLimit != 0) {
			remain += TickType(longIntervalCounterLimit - longIntervalCounter) * osTimer.getInterval();
		}
		return remain;
	}

	__forceinline void IRAM_ATTR setCallback(TimerCallback callback, void* arg)
	{
		this->callback.func = callback;
		this->callback.arg = arg;
	}

	__forceinline void setCallback(TimerDelegate delegateFunction)
	{
		delegate = delegateFunction;
		this->callback.func = nullptr;
	}

	void IRAM_ATTR setInterval(TickType interval);

	TickType IRAM_ATTR getInterval() const
	{
		TickType interval = osTimer.getInterval();
		if(longIntervalCounterLimit != 0) {
			interval *= longIntervalCounterLimit;
		}
		return interval;
	}

	__forceinline void IRAM_ATTR arm(bool repeating)
	{
		this->repeating = repeating;
		osTimer.arm(longIntervalCounterLimit || repeating);
	}

	__forceinline void IRAM_ATTR disarm()
	{
		osTimer.disarm();
		longIntervalCounter = 0;
	}

protected:
	void longTick();

private:
	OsTimerApi osTimer;
	struct {
		TimerCallback func = nullptr;
		void* arg = nullptr;
	} callback;
	TimerDelegate delegate; ///< User-provided callback delegate
	bool repeating = false;
	// Because of the limitation in Espressif SDK a workaround
	// was added to allow for longer timer intervals.
	uint16_t longIntervalCounter = 0;
	uint16_t longIntervalCounterLimit = 0;
};

template <class TimerClass> void OsTimer64Api<TimerClass>::setInterval(TickType interval)
{
	constexpr auto maxTicks = osTimer.maxTicks();
	if(interval > maxTicks) {
		// interval too large, calculate a good divider
		uint32_t div = (interval / (maxTicks + 1)) + 1; // integer division, intended

		// We will lose some precision here but its so small it won't matter.
		// Error will be microseconds mod div which can at most be div-1.
		// For small intervals (and thus small values of div) we are talking a few us.

		interval /= div;
		longIntervalCounterLimit = div;
	} else {
		longIntervalCounterLimit = 0;
	}
	longIntervalCounter = 0;
	osTimer.setInterval(interval);
}

template <class TimerClass> void OsTimer64Api<TimerClass>::longTick()
{
	if(longIntervalCounterLimit != 0) {
		longIntervalCounter++;

		if(longIntervalCounter < longIntervalCounterLimit) {
			return;
		}

		// For long intervals os_timer is set to repeating.
		// Stop timer if it was not a repeating timer.
		if(repeating) {
			longIntervalCounter = 0;
		} else {
			osTimer.disarm();
		}
	}

	if(callback.func != nullptr) {
		callback.func(callback.arg);
	} else if(delegate) {
		delegate();
	}

	static_cast<TimerClass*>(this)->expired();
}

/**
 * @brief Class template adding delegate callback method support to the basic CallbackTimer template
 */
template <typename TimerApi> class DelegateCallbackTimer : public CallbackTimer<TimerApi>
{
public:
	using typename TimerApi::TickType;
	using typename TimerApi::TimeType;
	using CallbackTimer<TimerApi>::initializeUs;
	using CallbackTimer<TimerApi>::initializeMs;
	using CallbackTimer<TimerApi>::setCallback;

	/** @brief  Initialise timer in microseconds, with static check
     *  @tparam microseconds Timer interval in microseconds
     *  @param  delegateFunction Function to call when timer triggers
     *  @retval ExtendedCallbackTimer& Reference to timer
     */
	template <TimeType microseconds> DelegateCallbackTimer& IRAM_ATTR initializeUs(TimerDelegate delegateFunction)
	{
		setCallback(delegateFunction);
		this->template setIntervalUs<microseconds>();
		return *this;
	}

	/** @brief  Initialise hardware timer in milliseconds, with static check
     *  @tparam milliseconds Timer interval in milliseconds
     *  @param  delegateFunction Function to call when timer triggers
     *  @retval ExtendedCallbackTimer& Reference to timer
     */
	template <uint32_t milliseconds> DelegateCallbackTimer& IRAM_ATTR initializeMs(TimerDelegate delegateFunction)
	{
		setCallback(delegateFunction);
		this->template setIntervalMs<milliseconds>();
		return *this;
	}

	/** @brief  Initialise millisecond timer
     *  @param  milliseconds Duration of timer in milliseconds
     *  @param  delegateFunction Function to call when timer triggers
     *  @note   Delegate callback method
     */
	DelegateCallbackTimer& initializeMs(uint32_t milliseconds, TimerDelegate delegateFunction)
	{
		setCallback(delegateFunction);
		this->setIntervalMs(milliseconds);
		return *this;
	}

	/** @brief  Initialise microsecond timer
     *  @param  microseconds Duration of timer in milliseconds
     *  @param  delegateFunction Function to call when timer triggers
     *  @note   Delegate callback method
     */
	DelegateCallbackTimer& initializeUs(uint32_t microseconds, TimerDelegate delegateFunction)
	{
		setCallback(delegateFunction);
		this->setIntervalUs(microseconds);
		return *this;
	}

	/** @brief	Set timer trigger function using Delegate callback method
	*  	@param	delegateFunction Function to be called on timer trigger
	*  	@note	Don't use this for interrupt timers
	*/
	void setCallback(TimerDelegate delegateFunction)
	{
		// Always disarm before setting the callback
		this->stop();
		TimerApi::setCallback(delegateFunction);
		this->callbackSet = bool(delegateFunction);
	}
};

/**
 * @brief Callback timer class
 */
class Timer : public DelegateCallbackTimer<OsTimer64Api<Timer>>
{
protected:
	friend OsTimer64Api<Timer>;

	void expired()
	{
	}
};

/**
 * @brief Auto-delete callback timer class
 */
class AutoDeleteTimer : public DelegateCallbackTimer<OsTimer64Api<AutoDeleteTimer>>
{
protected:
	friend OsTimer64Api<AutoDeleteTimer>;

	// Ensures object may only be created on the heap
	~AutoDeleteTimer()
	{
	}

	void expired()
	{
		delete this;
	}
};

/** @} */
