#include <SmingCore.h>
#include <Libraries/BMP180/BMP180.h>

BMP180 barometer;
void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Wire.begin();

	if(!barometer.EnsureConnected())
		Serial.println("Could not connect to BMP180.");

	// When we have connected, we reset the device to ensure a clean start.
	//barometer.SoftReset();
	// Now we initialize the sensor and pull the calibration data.
	barometer.Initialize();
	barometer.PrintCalibrationData();

	Serial.print("Start reading");

	// Retrive the current pressure in Pascals.
	long currentPressure = barometer.GetPressure();

	// Print out the Pressure.
	Serial.print("Pressure: ");
	Serial.print(currentPressure);
	Serial.print(" Pa");

	// Retrive the current temperature in degrees celcius.
	float currentTemperature = barometer.GetTemperature();

	// Print out the Temperature
	Serial.print("\tTemperature: ");
	Serial.print(currentTemperature);
	Serial.write(176);
	Serial.print("C");

	Serial.println(); // Start a new line.
}
