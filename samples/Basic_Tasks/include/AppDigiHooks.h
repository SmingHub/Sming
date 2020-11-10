#pragma once

#include <DigitalHooks.h>
#include <SignalGenerator.h>

class AppDigiHooks : public DigitalHooks
{
public:
	AppDigiHooks();

	uint16_t analogRead(uint16_t pin);

private:
	SignalGenerator gen;
};

extern AppDigiHooks appDigiHooks;
