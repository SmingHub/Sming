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
 * @see See IDF components/esp_common/src/ets_timer_legacy.c
 */
struct OsTimerClock : public NanoTime::Clock<OsTimerClock, HW_TIMER2_CLK, uint32_t, 0xFFFFFFFFU> {
	static constexpr const char* typeName()
	{
		return "OsTimerClock";
	}

	static uint32_t __forceinline ticks()
	{
		return hw_timer2_read();
	}
};

/**
 * @brief Clock implementation for polled timers
 * @note The Esp32 timer is actually 64-bit but for now stick with 32-bits for more efficient code.
 */
using PolledTimerClock = OsTimerClock;

using CpuCycleClockSlow = CpuCycleClock<eCF_80MHz>;
using CpuCycleClockNormal = CpuCycleClock<eCF_160MHz>;
#ifdef SUBARCH_ESP32C3
using CpuCycleClockFast = CpuCycleClockNormal;
#else
using CpuCycleClockFast = CpuCycleClock<eCF_240MHz>;
#endif

/** @} */
