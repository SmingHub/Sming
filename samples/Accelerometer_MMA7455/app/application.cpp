#include <SmingCore.h>
#include <Libraries/MMA_7455/MMA_7455.h>

// For more information read: https://code.google.com/p/mma-7455-arduino-library/
MMA_7455 accel;
Timer procTimer;

void readSensor()
{
	Serial.println("Reading..");

	int8_t x = accel.readAxis('x');
	int8_t y = accel.readAxis('y');
	int8_t z = accel.readAxis('z');
	Serial.print("Accelerometer data: ");
	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.print(", ");
	Serial.println(z);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial
	Serial.println("Starting...");

	// You can change pins:
	//Wire.pins(14, 12); // SDA, SCL
	Wire.begin();

	// Select the Working Mode
	accel.initSensitivity(MMA_7455_2G_MODE);

	// Start reading loop
	procTimer.initializeMs(300, readSensor).start();
}
