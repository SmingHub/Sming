#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "Libraries/AM2321/AM2321.h"

AM2321 am2321;

Timer procTimer;
bool state = true;

void read()
{
	Serial.print(am2321.read());
	Serial.print(",");
	Serial.print(am2321.temperature / 10.0);
	Serial.print(",");
	Serial.println(am2321.humidity / 10.0);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable/disable debug output

	// I2C
	pinMode(12, OUTPUT);
	digitalWrite(12, 1);
	delay(600);

	// Default I2C pins (SCL:0 , SDA: 2)

	// You can change pins:
	//Wire.pins(12, 14); // SCL, SDA

	am2321.begin(); // REQUIRED. Call it after choosing I2C pins.
	Serial.println(am2321.uid());

	procTimer.initializeMs(3000, read).start();
}
