/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Clocks.h
 *
 ****/

#pragma once

/** @ingroup system_clocks
 *  @{
 */

/**
 * @brief Clock implementation for os_timer API
 */
struct OsTimerClock : public NanoTime::Clock<OsTimerClock, 1000000U, uint32_t, 0xFFFFFFFFU> {
	static constexpr const char* typeName()
	{
		return "OsTimerClock";
	}

	static uint32_t __forceinline ticks()
	{
		return system_get_time();
	}
};

/**
 * @brief Clock implementation for polled timers
 * @note The Esp32 timer is actually 64-bit but for now stick with 32-bits for more efficient code.
 */
using PolledTimerClock = Timer2Clock;

using CpuCycleClockSlow = CpuCycleClock<eCF_80MHz>;
using CpuCycleClockNormal = CpuCycleClock<eCF_160MHz>;
#ifdef SOC_ESP32C3
using CpuCycleClockFast = CpuCycleClockNormal;
#else
using CpuCycleClockFast = CpuCycleClock<eCF_240MHz>;
#endif

/** @} */
