/* $Id: WMath.cpp 1156 2011-06-07 04:01:16Z bhagman $
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
|| @notes
|| | These functions have been defined in C++ so that we can utilize
|| | overloading.
||
|| @license Please see cores/Common/License.txt.
||
*/

#include "WiringFrameworkIncludes.h"


unsigned int static seed;

void srandom(unsigned int s)
{
	seed = s;
}

static int getRandom()
{
  unsigned int next = seed;
  int result;

  next *= 1103515245;
  next += 12345;
  result = (unsigned int) (next / 65536) % 2048;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  seed = next;

  return result;
}

void randomSeed(uint16_t seed)
{
  if (seed != 0)
	  srandom(seed);
}


long random(long howbig)
{
  if (howbig == 0)
    return 0;

  return getRandom() % howbig;
}


long random(long howsmall, long howbig)
{
  if (howsmall >= howbig)
    return howsmall;

  long diff = howbig - howsmall;
  return random(diff) + howsmall;
}


long map(long x, long in_min, long in_max, long out_min, long out_max) {
    long divisor = in_max - in_min;
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return ( (x - in_min) * (out_max - out_min) / divisor ) + out_min;
}


uint16_t makeWord(uint16_t w)
{
  return w;
}


uint16_t makeWord(uint8_t highByte, uint8_t lowByte)
{
  return (highByte << 8) | lowByte;
}

