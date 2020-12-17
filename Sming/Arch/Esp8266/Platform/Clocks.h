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

using OsTimerClock = Timer2Clock;

using PolledTimerClock = Timer2Clock;

using CpuCycleClockSlow = CpuCycleClock<eCF_80MHz>;
using CpuCycleClockNormal = CpuCycleClock<eCF_80MHz>;
using CpuCycleClockFast = CpuCycleClock<eCF_160MHz>;

/** @} */
