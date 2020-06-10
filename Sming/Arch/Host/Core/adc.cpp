#include <Digital.h>

uint16_t analogRead(uint16_t pin)
{
	if(pin == A0)
		return system_adc_read();
	else
		return -1; // Not supported
}
