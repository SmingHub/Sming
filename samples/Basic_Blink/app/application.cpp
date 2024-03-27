#include <SmingCore.h>

#ifdef ARCH_RP2040
#define LED_PIN PICO_DEFAULT_LED_PIN
#else
#define LED_PIN 2 // GPIO2
#endif

Timer procTimer;
bool state = true;

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void init()
{
	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs<1000>(blink).start();
}
