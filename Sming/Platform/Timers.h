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
 ****/

#pragma once

#include <PolledTimer.h>
#include "Clocks.h"

/** @ingroup polled_timer platform
 *  @{
*/

/*
 * Elapse timers
 */

template <NanoTime::Unit unit> using OneShotElapseTimer = PolledTimer::OneShot<Timer2Clock, unit>;
template <NanoTime::Unit unit> using PeriodicElapseTimer = PolledTimer::Periodic<Timer2Clock, unit>;

using OneShotFastMs = OneShotElapseTimer<NanoTime::Milliseconds>;
using OneShotFastUs = OneShotElapseTimer<NanoTime::Microseconds>;

using ElapseTimer = OneShotFastUs;

/*
 * CPU Cycle timers
 */

template <NanoTime::Unit units> using OneShotCpuCycleTimer = PolledTimer::OneShot<CpuCycleClockNormal, units>;
template <NanoTime::Unit units> using PeriodicCpuCycleTimer = PolledTimer::Periodic<CpuCycleClockNormal, units>;
template <NanoTime::Unit units> using OneShotCpuCycleTimerFast = PolledTimer::OneShot<CpuCycleClockFast, units>;
template <NanoTime::Unit units> using PeriodicCpuCycleTimerFast = PolledTimer::Periodic<CpuCycleClockFast, units>;
using CpuCycleTimer = OneShotCpuCycleTimer<NanoTime::Nanoseconds>;
using CpuCycleTimerFast = OneShotCpuCycleTimerFast<NanoTime::Nanoseconds>;

/*
 * Arduino-compatible types
 */

typedef OneShotFastMs oneShotFastMs SMING_DEPRECATED;
typedef OneShotFastUs oneShotFastUs SMING_DEPRECATED;
/** @} */
