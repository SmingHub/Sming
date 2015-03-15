/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/Clock.h"
#include "../Wiring/WiringFrameworkIncludes.h"

unsigned long millis(void)
{
	return system_get_time() / 1000UL;
}

unsigned long micros(void)
{
	return system_get_time();
}

void delay(uint32_t time)
{
	os_delay_us(time * 1000);
}

void delayMicroseconds(uint32_t time)
{
	os_delay_us(time);
}
