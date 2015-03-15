/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _NWTime_H_
#define _NWTime_H_

#include "../Wiring/WiringFrameworkDependencies.h"

unsigned long millis(void);
unsigned long micros(void);

void delay(uint32_t time);
#define delayMilliseconds(ms) delay(ms)
void delayMicroseconds(uint32_t time);

#endif
