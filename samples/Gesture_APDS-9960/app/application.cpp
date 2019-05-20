#include <SmingCore.h>

#include <Libraries/SparkFun_APDS9960/SparkFun_APDS9960.h>

SparkFun_APDS9960 apds = SparkFun_APDS9960();

// For I2C
// Default I2C pins 0 and 2. Pin 4 - interrupt pin
//
#define APDS9960_INT 4 // Needs to be an interrupt pin

void handleGesture()
{
	if(apds.isGestureAvailable()) {
		switch(apds.readGesture()) {
		case DIR_UP:
			Serial.println("UP");
			break;
		case DIR_DOWN:
			Serial.println("DOWN");
			break;
		case DIR_LEFT:
			Serial.println("LEFT");
			break;
		case DIR_RIGHT:
			Serial.println("RIGHT");
			break;
		case DIR_NEAR:
			Serial.println("NEAR");
			break;
		case DIR_FAR:
			Serial.println("FAR");
			break;
		default:
			Serial.println("NONE");
		}
	}
}

void interruptRoutine()
{
	detachInterrupt(APDS9960_INT);
	handleGesture();
	attachInterrupt(APDS9960_INT, InterruptDelegate(interruptRoutine), FALLING);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// WIFI not needed for demo. So disabling WIFI.
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	Serial.println();
	Serial.println("--------------------------------");
	Serial.println("SparkFun APDS-9960 - GestureTest");
	Serial.println("--------------------------------");

	// Initialize APDS-9960 (configure I2C and initial values)
	if(apds.init()) {
		Serial.println("APDS-9960 initialization complete");
	} else {
		Serial.println("Something went wrong during APDS-9960 init!");
	}

	// Start running the APDS-9960 gesture sensor engine
	if(apds.enableGestureSensor(true)) {
		Serial.println("Gesture sensor is now running");
	} else {
		Serial.println("Something went wrong during gesture sensor init!");
	}

	// Initialize interrupt service routine
	pinMode(APDS9960_INT, (GPIO_INT_TYPE)GPIO_PIN_INTR_ANYEDGE);
	attachInterrupt(APDS9960_INT, InterruptDelegate(interruptRoutine), FALLING);
}
