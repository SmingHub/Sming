/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ArduinoCompat.h - Arduino Compatibility Layer
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_ARDUINO_COMPAT_H_
#define _SMING_CORE_ARDUINO_COMPAT_H_

#include "SmingCore.h"
#include <stdio.h> ///< sprintf()

#ifdef __cplusplus
extern "C" {
#endif

#define abs(x) ((x) > 0 ? (x) : -(x))
#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))

void yield();

#ifdef __cplusplus
}
#endif

#endif /* _SMING_CORE_ARDUINO_COMPAT_H_ */
