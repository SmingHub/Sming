#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Ultrasonic/Ultrasonic.h>

#define LED_PIN 2 // GPIO2

Timer procTimer;
bool state = true;

Ultrasonic* ultrasonic;

void blink()
{
	float dist_cm = ultrasonic->Ranging(CM);       // get distance
	Serial.println(dist_cm);                      // print the distance

	//digitalWrite(LED_PIN, state);
	//state = !state;
}

void init()
{
//	pinMode(LED_PIN, OUTPUT);
	ultrasonic = new Ultrasonic(4, 5);
	procTimer.initializeMs(500, blink).start();
}
