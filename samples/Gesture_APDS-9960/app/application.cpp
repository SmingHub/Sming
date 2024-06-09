#include <SmingCore.h>

#include <SparkFun_APDS9960.h>

namespace
{
SparkFun_APDS9960 apds;

// For I2C
// Default I2C pins 0 and 2. Pin 4 - interrupt pin
//
#define APDS9960_INT 4

String dirToString(unsigned dir)
{
	switch(dir) {
	case DIR_NONE:
		return F("NONE");
	case DIR_LEFT:
		return F("LEFT");
	case DIR_RIGHT:
		return F("RIGHT");
	case DIR_UP:
		return F("UP");
	case DIR_DOWN:
		return F("DOWN");
	case DIR_NEAR:
		return F("NEAR");
	case DIR_FAR:
		return F("FAR");
	case DIR_ALL:
		return F("ALL");
	default:
		return nullptr;
	}
}

void handleGesture()
{
	if(!apds.isGestureAvailable()) {
		return;
	}

	auto dir = apds.readGesture();
	Serial.println(dirToString(dir));
}

void interruptDelegate()
{
	detachInterrupt(APDS9960_INT);
	handleGesture();
	attachInterrupt(APDS9960_INT, InterruptDelegate(interruptDelegate), FALLING);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// WIFI not needed for demo. So disabling WIFI.
#ifndef DISABLE_WIFI
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);
#endif

	Serial.print(_F("\r\n"
					"--------------------------------\r\n"
					"SparkFun APDS-9960 - GestureTest\r\n"
					"--------------------------------\r\n"));

	// Initialize APDS-9960 (configure I2C and initial values)
	if(!apds.init()) {
		Serial.println(_F("Something went wrong during APDS-9960 init!"));
		return;
	}

	Serial.println(_F("APDS-9960 initialization complete"));

	// Start running the APDS-9960 gesture sensor engine
	if(!apds.enableGestureSensor(true)) {
		Serial.println(_F("Something went wrong during gesture sensor init!"));
		return;
	}

	Serial.println(_F("Gesture sensor is now running"));

	// Initialize interrupt service routine
	pinMode(APDS9960_INT, GPIO_PIN_INTR_ANYEDGE);
	attachInterrupt(APDS9960_INT, InterruptDelegate(interruptDelegate), FALLING);
}
