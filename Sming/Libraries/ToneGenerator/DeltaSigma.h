/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DeltaSigma.h - Pulse-density modulation
 *
 * @author Sept 2019 mikee47 <mike@sillyhouse.net>
 *
 * https://en.wikipedia.org/wiki/Delta-sigma_modulation
 *
 ****/

#pragma once

#include <cstdint>

class DeltaSigma
{
public:
	/*
	 * Produce 32 bits of modulated output for an input sample
	 */
	uint32_t update(int16_t sample)
	{
		uint16_t DAC = 0x8000 + sample;
		uint32_t acc = 0;
		for(unsigned i = 0; i < 32; ++i) {
			acc <<= 1;
			if(DAC >= err) {
				acc |= 1;
				err += 0xFFFF - DAC;
			} else {
				err -= DAC;
			}
		}
		return acc;
	}

private:
	uint16_t err = 0;
};
