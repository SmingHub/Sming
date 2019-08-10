/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DigitalHooks.cpp
 *
 ****/

#include "DigitalHooks.h"
#include <hostlib/hostmsg.h>

void DigitalHooks::badPin(const char* function, uint16_t pin)
{
	host_printfp("BAD PIN (%u)\n", function, pin);
}

bool DigitalHooks::pinMode(uint16_t pin, uint8_t mode)
{
	host_printfp("%u, %u\n", __FUNCTION__, pin, mode);
	return true;
}

void DigitalHooks::digitalWrite(uint16_t pin, uint8_t val)
{
	host_printfp("%u, %u\n", __FUNCTION__, pin, val);
}

uint8_t DigitalHooks::digitalRead(uint16_t pin, uint8_t mode)
{
	host_printfp("%u, %u\n", __FUNCTION__, pin, mode);
	return 0;
}

void DigitalHooks::pullup(uint16_t pin, bool enable)
{
	host_printfp("%u, %u\n", __FUNCTION__, pin, enable);
}

unsigned long DigitalHooks::pulseIn(uint16_t pin, uint8_t state, unsigned long timeout)
{
	host_printfp("pin: %u, state: %u, timeout: %u\n", __FUNCTION__, pin, state, timeout);
	return 0;
}
