/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwareTimer.h
 *
 * Created 23.11.2015 by johndoe
 *
 ****/

/**	@ingroup callback_timer
 *  @{
 */

#pragma once

#include "CallbackTimer.h"
#include <Platform/Clocks.h>

/**	@ingroup callback_timer
 *  @{
 */

/**
 * @brief Hardware Timer interrupt mode
 */
enum HardwareTimerMode {
	eHWT_Maskable,
	eHWT_NonMaskable,
};

/**
 * @brief Class template for Timer1 API
 * @note Provides low-level interface for timer access
 */
template <hw_timer_clkdiv_t clkdiv, HardwareTimerMode mode>
class Timer1Api : public CallbackTimerApi<Timer1Api<clkdiv, mode>>
{
public:
	using Clock = Timer1Clock<clkdiv>;
	using TickType = uint32_t;
	using TimeType = uint32_t;

	static constexpr const char* typeName()
	{
		return "Timer1Api";
	}

	static constexpr TickType minTicks()
	{
		return Clock::template TimeConst<NanoTime::Microseconds, MIN_HW_TIMER1_INTERVAL_US>::ticks();
	}

	static constexpr TickType maxTicks()
	{
		return Clock::maxTicks();
	}

	static TickType ticks()
	{
		return Clock::ticks();
	}

	Timer1Api()
	{
		assert(state == eTS_CallbackNotSet);
	}

	~Timer1Api()
	{
		detach_interrupt();
	}

	__forceinline static void IRAM_ATTR setCallback(TimerCallback callback, void* arg)
	{
		if(callback == nullptr) {
			detach_interrupt();
		} else {
			assert(state <= eTS_Disarmed);
			hw_timer1_attach_interrupt(mode == eHWT_NonMaskable ? TIMER_NMI_SOURCE : TIMER_FRC1_SOURCE,
									   reinterpret_cast<hw_timer_callback_t>(callback), arg);
			state = eTS_Disarmed;
		}
	}

	__forceinline static void IRAM_ATTR setInterval(TickType interval)
	{
		Timer1Api::interval = interval;
	}

	__forceinline static TickType IRAM_ATTR getInterval()
	{
		return interval;
	}

	__forceinline static bool IRAM_ATTR isArmed()
	{
		return state >= eTS_Armed;
	}

	__forceinline static void IRAM_ATTR arm(bool repeating)
	{
		State newState = repeating ? eTS_ArmedAutoLoad : eTS_Armed;
		if(state != newState) {
			hw_timer1_enable(clkdiv, TIMER_EDGE_INT, repeating);
			state = newState;
		}
		hw_timer1_write(interval);
	}

	__forceinline static void IRAM_ATTR disarm()
	{
		if(isArmed()) {
			hw_timer1_disable();
			state = eTS_Disarmed;
		}
	}

private:
	__forceinline static void detach_interrupt()
	{
		disarm();
		if(state > eTS_CallbackNotSet) {
			hw_timer1_detach_interrupt();
			state = eTS_CallbackNotSet;
		}
	}

private:
	enum State {
		eTS_CallbackNotSet,
		eTS_Disarmed,
		eTS_Armed,
		eTS_ArmedAutoLoad,
	};
	static uint8_t state;
	static TickType interval;
};

template <hw_timer_clkdiv_t clkdiv, HardwareTimerMode mode> uint8_t Timer1Api<clkdiv, mode>::state;
template <hw_timer_clkdiv_t clkdiv, HardwareTimerMode mode> uint32_t Timer1Api<clkdiv, mode>::interval;

template <hw_timer_clkdiv_t clkdiv = TIMER_CLKDIV_16, HardwareTimerMode mode = eHWT_NonMaskable>

/**
 * @brief Hardware Timer class template with selectable divider and interrupt mode
 */
using HardwareTimer1 = CallbackTimer<Timer1Api<clkdiv, mode>>;

/**
 * @brief Default hardware Timer class
 */
using HardwareTimer = HardwareTimer1<>;

/**
 * @deprecated Use HardwareTimer class instead
 */
typedef HardwareTimer Hardware_Timer SMING_DEPRECATED;

/** @} */
