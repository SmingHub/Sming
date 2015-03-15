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

#ifndef WMATH_H
#define WMATH_H

#include "WiringFrameworkIncludes.h"

long random(long);
long random(long, long);
long map(long, long, long, long, long);
void randomSeed(uint16_t);
uint16_t makeWord(uint8_t, uint8_t);
uint16_t makeWord(uint16_t);

#endif
// WMATH_H
