/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Arduino Compatibility Layer
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef SMINGCORE_ARDUINOCOMPAT_H_
#define SMINGCORE_ARDUINOCOMPAT_H_

#include "SmingCore.h"

#ifdef __cplusplus
extern "C" {
#endif

#define abs(x) ((x) > 0 ? (x) : -(x))
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))

void yield();

#ifdef __cplusplus
}
#endif

#endif /* SMINGCORE_ARDUINOCOMPAT_H_ */
