/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 ****/

#include <Digital.h>
#include <WiringFrameworkIncludes.h>
#include "DigitalHooks.h"

// Wemos D1 mini has pin 16
#define PIN_MAX 16
static uint8 pinModes[PIN_MAX + 1];

DigitalHooks defaultHooks;
static DigitalHooks* activeHooks = &defaultHooks;

DigitalHooks* setDigitalHooks(DigitalHooks* hooks)
{
	DigitalHooks* current = activeHooks;
	activeHooks = hooks;
	return current;
}

static inline bool checkPin(const char* function, uint16_t pin)
{
	if(pin <= PIN_MAX) {
		return true;
	}

	if(activeHooks != nullptr) {
		activeHooks->badPin(function, pin);
	}

	return false;
}

void pinMode(uint16_t pin, uint8_t mode)
{
	if(checkPin(__FUNCTION__, pin)) {
		if(activeHooks != nullptr && !activeHooks->pinMode(pin, mode)) {
			return; // Don't change mode
		}
		pinModes[pin] = mode;
	}
}

//Detect if pin is input
bool isInputPin(uint16_t pin)
{
	return checkPin(__FUNCTION__, pin) ? pinModes[pin] : false;
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	if(checkPin(__FUNCTION__, pin) && activeHooks != nullptr) {
		activeHooks->digitalWrite(pin, val);
	}
}

uint8_t digitalRead(uint16_t pin)
{
	if(checkPin(__FUNCTION__, pin) && activeHooks != nullptr) {
		return activeHooks->digitalRead(pin, pinModes[pin]);
	}

	return 0;
}

void pullup(uint16_t pin)
{
	if(checkPin(__FUNCTION__, pin) && activeHooks != nullptr) {
		activeHooks->pullup(pin, true);
	}
}

void noPullup(uint16_t pin)
{
	if(checkPin(__FUNCTION__, pin) && activeHooks != nullptr) {
		activeHooks->pullup(pin, false);
	}
}

unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout)
{
	if(checkPin(__FUNCTION__, pin) && activeHooks != nullptr) {
		return activeHooks->pulseIn(pin, state, timeout);
	} else {
		return 0;
	}
}
