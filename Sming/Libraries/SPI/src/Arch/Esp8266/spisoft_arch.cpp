/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * spisoft_arch.cpp
 *
 ****/

#include <SPISettings.h>
#include <FlashString/Array.hpp>
#include <esp_system.h>

#if defined(SPISOFT_DELAY_VARIABLE)

namespace
{
/*
 * Matching small delay values to frequency reuires a lookup table as the relationship is non-linear.
 * Given values are clock period in ns, found by measurement.
 *
 * Two tables are required depending on currently selected CPU frequency.
 */

DEFINE_FSTR_ARRAY(table80, uint16_t, 737, 760, 823, 886, 948, 1011, 1073, 1135)
DEFINE_FSTR_ARRAY(table160, uint16_t, 469, 470, 475, 511, 537, 573, 600, 636)

/*
 * Longer delay values are calculated, these are linearly proportional to period.
 */

struct CoEfficient {
	int16_t m;
	int16_t c;
};

constexpr int M{16}; // Scalar to store values to one DP

constexpr CoEfficient coefficients[2]{
	{int16_t(124.9 * M), 224},
	{int16_t(62.5 * M), 175},
};

} // namespace

uint8_t checkSpeed(SPISpeed& speed)
{
	constexpr int maxDelay{255};

	union SpeedConfig {
		uint32_t val;
		struct {
			uint8_t cpuFreq;
			uint8_t delay;
		};
	};

	SpeedConfig cfg{speed.regVal};

	// A change in CPU frequency or requested frequency will fail this check
	auto cpuFreq = system_get_cpu_freq();
	if(cfg.cpuFreq == cpuFreq) {
		// Already calculated, done
		return cfg.delay;
	}

	constexpr int NS = 1000000000;

	auto periodRequired = NS / speed.frequency;
	const FSTR::Array<uint16_t>& table = (cpuFreq <= 80) ? table80 : table160;
	int delay = table.length() - 1;
	uint16_t period = table[delay];
	if(period == periodRequired) {
		// OK, entry matches
	} else if(period > periodRequired) {
		// Value is in table, match highest frequency not exceeding requested value
		for(uint8_t i = 0; i < table.length(); ++i) {
			period = table[i];
			if(period >= periodRequired) {
				delay = i;
				break;
			}
		}
	} else {
		// Calculate the delay, and resulting clock frequency
		auto coeff = coefficients[(cpuFreq <= 80) ? 0 : 1];
		if(periodRequired == 0) {
			delay = maxDelay;
		} else {
			delay = M * (periodRequired - coeff.c) / coeff.m;
			delay = std::min(delay, maxDelay);
		}
#ifdef SPI_DEBUG
		period = (coeff.m * delay / M) + coeff.c;
#endif
	}

	cfg.cpuFreq = cpuFreq;
	cfg.delay = delay;
	speed.regVal = cfg.val;

#ifdef SPI_DEBUG
	debugf("[SSPI] Using delay %u -> target freq %u -> result %u", delay, speed.frequency, NS / period);
#endif

	return cfg.delay;
}

#endif
