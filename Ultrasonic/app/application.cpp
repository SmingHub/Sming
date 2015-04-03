/**
 * WARNING !
 * Ultrasonic modules usually work with 5v power and TTL levels,
 * so You need voltage divider or level shifter for ECHO pin.
 *
 * Trigger pin is tolerant for 3.3v and in my case work without problems
 *
 * By nik.sharky http://esp8266.ru/forum/members/sharky.396/
 */

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Ultrasonic/Ultrasonic.h>

#define TRIG_PIN 4
#define ECHO_PIN 5

Timer procTimer;

Ultrasonic* ultrasonic;

void measure()
{
	float dist_cm = ultrasonic->Ranging(CM);       // get distance
	Serial.println(dist_cm);                      // print the distance
}

void init()
{
	ultrasonic = new Ultrasonic(TRIG_PIN, ECHO_PIN);
	procTimer.initializeMs(500, measure).start();
}
