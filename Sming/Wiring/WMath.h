/* $Id: WMath.h 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | Math methods.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#pragma once

#include <cstdint>

long random(long howbig);

long random(long howsmall, long howbig);

long map(long x, long in_min, long in_max, long out_min, long out_max);

void randomSeed(uint16_t);

static inline uint16_t makeWord(uint8_t highByte, uint8_t lowByte)
{
	return (highByte << 8) | lowByte;
}

static inline uint16_t makeWord(uint16_t w)
{
	return w;
}
