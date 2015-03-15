/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "Digital.h"
#include "../SmingCore/ESP8266EX.h"

#include "../SmingCore/Digital.h"

void EspDigitalPin::mode(uint8_t mode)
{
	pinMode(id, mode);
}

void EspDigitalPin::write(uint8_t val)
{
	digitalWrite(id, val);
}

uint8_t EspDigitalPin::read()
{
	return digitalRead(id);
}
