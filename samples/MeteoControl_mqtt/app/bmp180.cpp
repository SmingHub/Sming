#include <SmingCore.h>
#include <Libraries/BMP180/BMP180.h>

#include "configuration.h"

//    GPIO0 SCL
//    GPIO2 SDA

namespace
{
BMP180 barometer;
SimpleTimer publishBMPTimer;

void publishBMP()
{
	if(mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.print(_F("*********************************************"));
	Serial.println(_F("Start reading BMP180 sensor"));
	if(!barometer.EnsureConnected()) {
		Serial.println(_F("Could not connect to BMP180"));
		return;
	}
	// Retrieve the current pressure in Pascals
	long currentPressure = barometer.GetPressure();
	// convert pressure to mmHg
	float BMPPress = currentPressure / 133.322;

	// Print out the Pressure
	Serial << _F("Pressure: ") << BMPPress << _F(" mmHg") << endl;
	mqtt.publish(BMP_P, String(BMPPress));

	// Retrieve the current temperature in degrees celsius
	float BMPTemp = barometer.GetTemperature();

	// Print out the Temperature
	Serial << _F("Temperature: ") << BMPTemp << " °C" << endl;
	mqtt.publish(BMP_T, String(BMPTemp));
	Serial.println(_F("BMP180 sensor read and transmitted to server\r\n"
					  "********************************************"));
}

} // namespace

void BMPinit()
{
	// When we have connected, we reset the device to ensure a clean start.
	barometer.SoftReset();

	// Now we initialize the sensor and pull the calibration data
	barometer.Initialize();
	barometer.PrintCalibrationData();

	publishBMPTimer.initializeMs<TIMER * 3000>(publishBMP).start(); // start publish BMP180 sensor data
}
