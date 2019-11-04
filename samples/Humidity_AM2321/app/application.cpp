#include <SmingCore.h>
#include "Libraries/AM2321/AM2321.h"

AM2321 am2321;

Timer procTimer;
bool state = true;

// You can change I2C pins here:
const int SCL = 5;
const int SDA = 4;

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

	// Switch AM2321 sensor to I2C mode
	pinMode(SCL, OUTPUT);
	digitalWrite(SCL, HIGH);
	delay(500);

	// Apply I2C pins
	Wire.pins(SDA, SCL);
	Wire.begin();

	am2321.begin(); // REQUIRED. Call it after choosing I2C pins.
	Serial.println(am2321.uid());

	procTimer.initializeMs(3000, read).start();
}
