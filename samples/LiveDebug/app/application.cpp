#include <user_config.h>
#include "SmingCore.h"
#include "HardwareTimer.h"

#define LED_PIN 2 // GPIO2

/*
 * This example uses the hardware timer for best timing accuracy. There is only one of these on the ESP8266,
 * so it may not be available if another module requires it.
 * Most timing applicatons can use a SimpleTimer, which is good for intervals of up to about 268 seconds.
 * For longer intervals, use a Timer.
 */
HardwareTimer procTimer;
bool state = true;

/*
* Notice:  Software breakpoints work only on code that is in RAM. 
*          In Sming you have to use the GDB_IRAM_ATTR to do this.
*/
void GDB_IRAM_ATTR blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void GDB_IRAM_ATTR init()
{
	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(1000, blink).start();
}
