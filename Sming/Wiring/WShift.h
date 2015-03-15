/* $Id: WShift.h 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | Bit shifting methods.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef WSHIFT_H
#define WSHIFT_H

#include "WiringFrameworkIncludes.h"

uint16_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t count = 8, uint8_t delayTime = 1);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint16_t value, uint8_t count = 8, uint8_t delayTime = 1);

#endif
// WSHIFT_H
