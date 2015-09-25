#include <user_config.h>
#include <SmingCore/SmingCore.h>

#define LED_PIN15 15 // GPIO2
#define LED_PIN02 2 // GPIO2
#define LED_PIN00 0 // GPIO2

Timer procTimer;
bool state = true;

void blink1()
{
	digitalWrite(LED_PIN15, state);
	state = !state;
}

void blink2()
{
	digitalWrite(LED_PIN02, state);
	state = !state;
}


void blink3()
{
	digitalWrite(LED_PIN00, state);
	state = !state;
}


void init()
{
	pinMode(LED_PIN15, OUTPUT);
	pinMode(LED_PIN02, OUTPUT);
	pinMode(LED_PIN00, OUTPUT);
	procTimer.initializeMs(100, blink1).start();
	procTimer.initializeMs(200, blink2).start();
	procTimer.initializeMs(300, blink3).start();
}
