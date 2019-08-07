/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CycleCounter.h - Class to measure elapesd time periods using CPU cycle counter.
 *
 * Intended purpose is to evaluate code performance and possibly for _very_ short
 * time interval requirements.
 *
 * Note that the CPU frequency may not be constant. If you require accurate time
 * period evaluation see `ElapseTimer`.
 *
 ****/
#pragma once

#include <stdint.h>
#include <sming_attr.h>

#ifdef ARCH_HOST
#include <x86intrin.h>
#endif

/**
 * @brief Class for counting CPU cycles
 */
class CycleCounter
{
public:
	__forceinline CycleCounter()
	{
		start();
	}

	/** @brief Get the current CPU cycle count */
	static uint32_t __forceinline count()
	{
#ifdef ARCH_HOST
		return __rdtsc();
#else
		volatile uint32_t ccount;
		__asm__ volatile("rsr %0, ccount" : "=r"(ccount));
		return ccount;
#endif
	}

	/** @brief Reset the start position to the current cycle count */
	void __forceinline start()
	{
		startCount = count();
	}

	/** @brief Get elapsed cycle count since start() was last called */
	uint32_t __forceinline elapsed()
	{
		return count() - startCount;
	}

private:
	uint32_t startCount;
};
