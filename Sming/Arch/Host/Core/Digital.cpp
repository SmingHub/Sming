/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 ****/

#include "Digital.h"
#include "WiringFrameworkIncludes.h"

#define PIN_MAX 16
static uint8 pinModes[PIN_MAX];

static inline bool checkPin(uint16_t pin)
{
	if(pin < 16) {
		return true;
	} else {
		hostmsg("BAD PIN %u", pin);
		return false;
	}
}

void pinMode(uint16_t pin, uint8_t mode)
{
	if(checkPin(pin)) {
		pinModes[pin] = mode;
		hostmsg("pinMode(%u, %u)", pin, mode);
	}
}

//Detect if pin is input
bool isInputPin(uint16_t pin)
{
	return checkPin(pin) ? pinModes[pin] : false;
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	checkPin(pin);
	hostmsg("digitalWrite(%u, %u)", pin, val);
}

uint8_t digitalRead(uint16_t pin)
{
	checkPin(pin);
	hostmsg("digitalRead(%u)", pin);
	return 0;
}

void pullup(uint16_t pin)
{
	checkPin(pin);
	hostmsg("pullup(%u)", pin);
}

void noPullup(uint16_t pin)
{
	checkPin(pin);
	hostmsg("noPullup(%u)", pin);
}

unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout)
{
	checkPin(pin);
	hostmsg("pulseIn(%u, %u, %lu)", pin, state, timeout);
	return 0;
}
