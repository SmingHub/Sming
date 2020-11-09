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
	if(checkPin(__FUNCTION__, pin)) {
		switch(pinModes[pin]) {
		case INPUT:
		case INPUT_PULLUP:
		case INPUT_PULLDOWN_16:
		case WAKEUP_PULLUP:
		case WAKEUP_PULLDOWN:
			return true;
		}
	}

	return false;
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

uint16_t analogRead(uint16_t pin)
{
	return (activeHooks == nullptr) ? 0 : activeHooks->analogRead(pin);
}
