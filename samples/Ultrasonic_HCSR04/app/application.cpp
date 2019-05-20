/**
 * WARNING !
 * Ultrasonic_HCSR04 modules usually work with 5v power and TTL levels,
 * so You need voltage divider or level shifter for ECHO pin.
 *
 * Trigger pin is tolerant for 3.3v and in my case work without problems
 *
 * By nik.sharky http://esp8266.ru/forum/members/sharky.396/
 */

#include <SmingCore.h>
#include <Libraries/Ultrasonic/Ultrasonic.h>

#define TRIG_PIN 2
#define ECHO_PIN 5

Timer procTimer;
Ultrasonic ultrasonic = Ultrasonic();

void measure()
{
	// get distance
	uint16_t dist = ultrasonic.rangeCM();

	// print the distance
	Serial.println(dist);
}

void init()
{
	ultrasonic.begin(TRIG_PIN, ECHO_PIN);
	procTimer.initializeMs(500, measure).start();
}
