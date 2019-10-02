#pragma once

#include <Platform/Timers.h>
#include "MinMax.h"

namespace Profiling
{
template <class Timer> class MinMaxTimes : public MinMax32, public Timer
{
public:
	MinMaxTimes(const String& title) : MinMax32(title)
	{
	}

	__forceinline void update()
	{
		MinMax32::update(this->elapsedTicks());
	}

	NanoTime::Time<uint32_t> getMinTime() const
	{
		return this->template ticksToTime(getMin());
	}

	NanoTime::Time<uint32_t> getMaxTime() const
	{
		return this->template ticksToTime(getMax());
	}

	NanoTime::Time<uint32_t> getAverageTime() const
	{
		return this->template ticksToTime(getAverage());
	}

	NanoTime::Time<uint32_t> getTotalTime() const
	{
		return this->template ticksToTime(getTotal());
	}

	size_t printTo(Print& p) const
	{
		auto res = p.print(getTitle());
		res += p.print(": count=");
		res += p.print(getCount());
		res += p.print(", total=");
		res += p.print(getTotalTime().toString());
		res += p.print(", min=");
		res += p.print(getMinTime().toString());
		res += p.print(", max=");
		res += p.print(getMaxTime().toString());
		res += p.print(", average=");
		res += p.print(getAverageTime().toString());
		return res;
	}
};

using CpuCycleTimes = MinMaxTimes<CpuCycleTimer>;
using MilliTimes = MinMaxTimes<OneShotFastMs>;
using MicroTimes = MinMaxTimes<OneShotFastUs>;

} // namespace Profiling
