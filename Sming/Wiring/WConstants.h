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

#include <user_config.h>

// Wiring API version for libraries
// this is passed in at compile-time
#ifndef WIRING
#define WIRING 101
#endif

// passed in at compile-time
#ifndef F_CPU
#define F_CPU 80000000L
#warning "F_CPU was not defined.  Default to 80 MHz."
#endif

/*************************************************************
 * Constants
 *************************************************************/

#define LOW      0x0
#define HIGH     0x1
//#define HIGH     0xFF

//GPIO FUNCTIONS
#define INPUT             0x00
#define INPUT_PULLUP      0x02
#define INPUT_PULLDOWN_16 0x04 // PULLDOWN only possible for pin16
#define OUTPUT            0x01
#define OUTPUT_OPEN_DRAIN 0x03
#define WAKEUP_PULLUP     0x05
#define WAKEUP_PULLDOWN   0x07
#define SPECIAL           0xF8 //defaults to the usable BUSes uart0rx/tx uart1tx and hspi
#define FUNCTION_0        0x08
#define FUNCTION_1        0x18
#define FUNCTION_2        0x28
#define FUNCTION_3        0x38
#define FUNCTION_4        0x48

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
//#ifndef min
//#define min(a,b)                       ((a)<(b)?(a):(b))
//#endif
//#ifndef max
//#define max(a,b)                       ((a)>(b)?(a):(b))
//#endif
//#define round(x)                       ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg)                   ((deg)*DEG_TO_RAD)
#define degrees(rad)                   ((rad)*RAD_TO_DEG)
#define constrain(amt,low,high)        ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

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
