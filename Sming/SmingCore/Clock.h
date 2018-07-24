/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/
/** @defgroup   timedelay Time and Delay
 *  @brief      Provides time and delay functions
 *  @ingroup    datetime
 *  @{
*/

#ifndef _NWTime_H_
#define _NWTime_H_

#include "../Wiring/WiringFrameworkDependencies.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief  Get the system (up)time in milliseconds
 *  @retval "unsigned long" Quantity of milliseconds elapsed since clock epoch
 *  @note   Clock epoch will reset every 49 days, 17 hours, 2 minutes, 47 seconds, 296 milliseconds
 *  @note   This function uses ESP8266 _system time_ clock which pauses during sleep. Function is provided for compatibility with Arduino. For date and time functionality, use SystemClock
 *  @see    SystemClockClass
 */
unsigned long millis(void) __attribute__((weak));

/** @brief  Get the time from clock in microseconds
 *  @retval "unsigned long" Quantity of microseconds elapsed since clock epoch
 *  @note   Clock epoch will reset every 71 minutes, 47 seconds, 967296 microseconds
 *  @note   This function uses ESP8266 _system time_ clock which pauses during sleep. Function is provided for compatibility with Arduino. For date and time functionality, use SystemClock
 *  @see    SystemClockClass
 */
unsigned long micros(void) __attribute__((weak));

/** @brief  Pause execution
 *  @param  time Duration of delay in milliseconds
 */
void delay(uint32_t time);

/** @brief  Pause execution
 *  @param  ms Duration of delay in milliseconds
 */
#define delayMilliseconds(ms) delay(ms)

/** @brief  Pause execution
 *  @param  time Duration of delay in microseconds
 */
void delayMicroseconds(uint32_t time);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
