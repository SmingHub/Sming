#include <SmingCore.h>
#include <Libraries/BMP180/BMP180.h>

BMP180 barometer;

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Wire.begin();

	if(!barometer.EnsureConnected())
		Serial.println(_F("Could not connect to BMP180."));

	// When we have connected, we reset the device to ensure a clean start.
	//barometer.SoftReset();
	// Now we initialize the sensor and pull the calibration data.
	barometer.Initialize();
	barometer.PrintCalibrationData();

	Serial.print(_F("Start reading"));

	Serial << _F("Pressure: ") << barometer.GetPressure() << " Pa" << _F("\tTemperature: ")
		   << barometer.GetTemperature() << " °C" << endl;
}
