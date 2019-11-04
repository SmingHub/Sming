/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Clocks.h
 *
 * @author mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <NanoTime.h>
#include <driver/hw_timer.h>
#include <esp_clk.h>
#include "System.h"

/** @ingroup system_clocks
 *  @{
 */

/**
 * @brief Clock implementation for Hardware Timer 1
 * @tparam clkdiv Prescaler in use
 */
template <hw_timer_clkdiv_t clkdiv>
struct Timer1Clock
	: public NanoTime::Clock<Timer1Clock<clkdiv>, HW_TIMER_BASE_CLK / (1 << clkdiv), uint32_t, MAX_HW_TIMER1_INTERVAL> {
	static constexpr uint32_t prescale()
	{
		return 1 << clkdiv;
	}

	static constexpr const char* typeName()
	{
		return "Timer1Clock";
	}

	/*
	 * As Timer1 is a down-counter, when active this will indicate remaining ticks until the next interrupt.
	 */
	static uint32_t __forceinline ticks()
	{
		return hw_timer1_read();
	}
};

/**
 * @brief Clock implementation for Hardware Timer 2
 * @note Prescaler is fixed
 * @see See `driver/hw_timer.h`
 */
struct Timer2Clock : public NanoTime::Clock<Timer2Clock, HW_TIMER2_CLK, uint32_t, 0xFFFFFFFFU> {
	static constexpr const char* typeName()
	{
		return "Timer2Clock";
	}

	static uint32_t __forceinline ticks()
	{
		return NOW();
	}
};

/**
 * @brief Clock implementation for CPU cycle times
 * @note The intended purpose is to evaluate code performance and possibly for _very_ short
 * time interval requirements. The 32-bit counter overflows:
 *
 * 		Overflows every 53.687 seconds @ 80MHz CPU clock
 * 		Overflows every 26.843 seconds @ 160MHz CPU clock
 *
 * Generally this will provide the shortest time periods.
 *
 * Time calculations are fixed at compile time, so you should use either
 * CpuCycleClockNormal or CpuCycleClockFast depending on the currently configured CPU speed.
 * @see See `System::setCpuFrequency()`
 */
template <CpuFrequency cpuFreq>
struct CpuCycleClock
	: public NanoTime::Clock<CpuCycleClock<cpuFreq>, uint32_t(cpuFreq) * 1000000, uint32_t, 0xFFFFFFFF> {
	static constexpr const char* typeName()
	{
		return "CpuCycleClock";
	}

	static uint32_t __forceinline ticks()
	{
		return esp_get_ccount();
	}

	static constexpr bool isFast()
	{
		return cpuFreq == eCF_160MHz;
	}
};

/**
 * @brief
 */
using CpuCycleClockNormal = CpuCycleClock<eCF_80MHz>;

/**
 * @brief
 */
using CpuCycleClockFast = CpuCycleClock<eCF_160MHz>;

/** @} */
