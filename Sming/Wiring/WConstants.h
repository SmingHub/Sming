/* $Id: WConstants.h 1156 2011-06-07 04:01:16Z bhagman $
||
|| @author         Hernando Barragan <b@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
|| @contribution   Alexander Brevig <abrevig@wiring.org.co>
||
|| @description
|| | Main constant and macro definitions for Wiring.
|| |
|| | Wiring Common API
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#ifndef WCONSTANTS_H
#define WCONSTANTS_H

#include "../include/user_config.h"

// Wiring API version for libraries
// this is passed in at compile-time
#ifndef WIRING
#define WIRING 101
#endif

// passed in at compile-time
#ifndef F_CPU
#define F_CPU 16000000L
#warning "F_CPU was not defined.  Default to 16 MHz."
#endif

/*************************************************************
 * Constants
 *************************************************************/

#define LOW      0x0
#define HIGH     0x1
//#define HIGH     0xFF

#define INPUT    0x0
#define OUTPUT   0x1
//#define OUTPUT   0xFF

#define CHANGE   32 // to avoid conflict with HIGH value
#define FALLING  2
#define RISING   3

#define LSBFIRST 0x0
#define MSBFIRST 0x1

// Defined in ctypes
//#define true     0x1
//#define false    0x0
//#define TRUE     0x1
//#define FALSE    0x0
#define null     NULL

#define DEC      10
#define HEX      16
#define OCT      8
#define BIN      2
#define BYTE     0

#define PI                             (3.1415926535897932384626433832795)
#define TWO_PI                         (6.283185307179586476925286766559)
#define HALF_PI                        (1.5707963267948966192313216916398)
#define EPSILON                        (0.0001)
#define DEG_TO_RAD                     (0.017453292519943295769236907684886)
#define RAD_TO_DEG                     (57.295779513082320876798154814105)


/*************************************************************
 * Digital Constants
 *************************************************************/

#define PORT0 0
#define PORT1 1
#define PORT2 2
#define PORT3 3
#define PORT4 4
#define PORT5 5
#define PORT6 6
#define PORT7 7
#define PORT8 8
#define PORT9 9


/*************************************************************
 * Useful macros
 *************************************************************/

/*#define int(x)                         ((int)(x))
#define char(x)                        ((char)(x))
#define long(x)                        ((long)(x))
#define byte(x)                        ((uint8_t)(x))
#define float(x)                       ((float)(x))
#define boolean(x)                     ((uint8_t)((x)==0?false:true))
*/

#define word(...) makeWord(__VA_ARGS__)

#define sq(x)                          ((x)*(x))
//#define abs(x)                         ((x)>0?(x):-(x))
#define min(a,b)                       ((a)<(b)?(a):(b))
#define max(a,b)                       ((a)>(b)?(a):(b))
//#define round(x)                       ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg)                   ((deg)*DEG_TO_RAD)
#define degrees(rad)                   ((rad)*RAD_TO_DEG)
#define constrain(amt,low,high)        ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define bit(x)                         (1UL<<(x))
#define setBits(x, y)                  ((x)|=(y))
#define clearBits(x, y)                ((x)&=(~(y)))
//#define setBit(x, y)                   setBits((x), (bit((y))))
//#define clearBit(x, y)                 clearBits((x), (bit((y))))

#define bitsSet(x,y)                   (((x) & (y)) == (y))
#define bitsClear(x,y)                 (((x) & (y)) == 0)

#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define lowByte(x)                     ((uint8_t) ((x) & 0x00ff))
#define highByte(x)                    ((uint8_t) ((x)>>8))


#define clockCyclesPerMicrosecond()    (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a)   ((a) / clockCyclesPerMicrosecond())
#define microsecondsToClockCycles(a)   ((a) * clockCyclesPerMicrosecond())



/*************************************************************
 * Typedefs
 *************************************************************/

typedef unsigned int word;
typedef uint8_t byte;
typedef uint8_t boolean;
typedef void (*voidFuncPtr)(void);

#endif
// WCONSTANTS_H
