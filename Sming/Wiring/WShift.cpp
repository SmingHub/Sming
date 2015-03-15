/* $Id: WShift.cpp 1156 2011-06-07 04:01:16Z bhagman $
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

#include "../SmingCore/Clock.h"
#include "../SmingCore/Digital.h"
#include "WiringFrameworkIncludes.h"


uint16_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t count, uint8_t delayTime)
{
  uint16_t value = 0;

  for (uint8_t i = 0; i < count; ++i)
  {
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(delayTime);
    if (bitOrder == LSBFIRST)
      value |= digitalRead(dataPin) << i;
    else
      value |= digitalRead(dataPin) << ((count - 1) - i);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(delayTime);
  }
  return value;
}


void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint16_t val, uint8_t count, uint8_t delayTime)
{
  int i;

  for (i = 0; i < count; ++i)
  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else
      digitalWrite(dataPin, !!(val & (1 << ((count - 1) - i))));

    digitalWrite(clockPin, HIGH);
    delayMicroseconds(delayTime);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(delayTime);
  }
}
