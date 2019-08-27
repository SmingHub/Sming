#pragma once

#include <Services/Profiling/CycleCounter.h>
#include <Services/Profiling/MinMax.h>

using MinMax32 = Profiling::MinMax<uint32_t>;

class CycleTimes : public MinMax32
{
public:
	CycleTimes(const String& title) : MinMax32(title)
	{
	}

	__forceinline void start()
	{
		timer.start();
	}

	__forceinline void IRAM_ATTR update()
	{
		MinMax32::update(timer.elapsed());
	}

private:
	CycleCounter timer;
};
