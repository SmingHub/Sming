#include <AppDigiHooks.h>
#include <pins_arduino.h>
#include <math.h>

AppDigiHooks appDigiHooks;

namespace
{
// ADC has 10-bit resolution
const unsigned ADC_MAX_VALUE = 1023;

}; // namespace

AppDigiHooks::AppDigiHooks()
{
	gen.frequency = 20000.0;
	gen.signalType = eST_Square;
}

uint16_t AppDigiHooks::analogRead(uint16_t pin)
{
	if(pin == A0) {
		delayMicroseconds(50);
		return round((gen.getValue() + 1) * ADC_MAX_VALUE / 2.0);
	}

	return DigitalHooks::analogRead(pin);
}
