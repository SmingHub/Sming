/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/Clock.h"
#include "../Wiring/WiringFrameworkIncludes.h"

#define MAX_SAFE_DELAY 1000

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
	int quotient = time / MAX_SAFE_DELAY;
	int remainder = time % MAX_SAFE_DELAY;
	for(int i=0, max = quotient + 1; i < max ; i++) {
		if(i == quotient) {
			os_delay_us(remainder * 1000);
		}
		else {
			os_delay_us(MAX_SAFE_DELAY * 1000);
		}

		system_soft_wdt_feed ();
	}
}

void delayMicroseconds(uint32_t time)
{
	os_delay_us(time);
}
