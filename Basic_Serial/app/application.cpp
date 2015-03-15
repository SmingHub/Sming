#include <user_config.h>
#include <SmingCore/SmingCore.h>

Timer procTimer;

void sayHello()
{
	Serial.println("Hello Sming! Let's do smart things.");
	Serial.print("time: ");
	Serial.println(micros());
	Serial.println();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 230400 by default
	procTimer.initializeMs(500, sayHello).start();
}
