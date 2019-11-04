/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ArduinoCompat.h - Arduino Compatibility Layer
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

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
