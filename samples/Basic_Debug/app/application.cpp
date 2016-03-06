#include <user_config.h>
#include <SmingCore/SmingCore.h>

#define LED_PIN 2 // GPIO2

Timer procTimer;
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
