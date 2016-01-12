/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _NWDigital_H_
#define _NWDigital_H_

#include "../SmingCore/ESP8266EX.h"
#include "../Wiring/WiringFrameworkDependencies.h"

//#define interrupts()   sei()
//#define noInterrupts() cli()

void pinMode(uint16_t pin, uint8_t mode);

void IRAM_ATTR digitalWrite(uint16_t pin, uint8_t val);
uint8_t IRAM_ATTR digitalRead(uint16_t pin);

void IRAM_ATTR pullup(uint16_t pin);
void IRAM_ATTR noPullup(uint16_t pin);

bool IRAM_ATTR isInputPin(uint16_t pin);

unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout =
		1000000L);
#endif
