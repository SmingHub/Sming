#ifndef INCLUDE_BMP180_H_
#define INCLUDE_BMP180_H_

#include <SmingCore.h>
#include <Libraries/BMP180/BMP180.h>

#include "configuration.h"

//    GPIO0 SCL
//    GPIO2 SDA

BMP180 barometer;

Timer publishBMPTimer;

void publishBMP()
{
	if(mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.print("*********************************************");
	Serial.println("Start reading BMP180 sensor");
	if(!barometer.EnsureConnected()) {
		Serial.println("Could not connect to BMP180");
	} else {
		// Retrieve the current pressure in Pascals
		long currentPressure = barometer.GetPressure();
		// convert pressure to mmHg
		float BMPPress = currentPressure / 133.322;

		// Print out the Pressure
		Serial.print("Pressure: ");
		Serial.print(BMPPress);
		Serial.println(" mmHg");
		mqtt.publish(BMP_P, String(BMPPress));

		// Retrieve the current temperature in degrees celsius
		float BMPTemp = barometer.GetTemperature();

		// Print out the Temperature
		Serial.print("Temperature: ");
		Serial.print(BMPTemp);
		Serial.println(" *C");
		mqtt.publish(BMP_T, String(BMPTemp));
		Serial.println("BMP180 sensor read and transmitted to server");
		Serial.println("*********************************************");
	}
}

void BMPinit()
{
	// When we have connected, we reset the device to ensure a clean start.
	barometer.SoftReset();

	// Now we initialize the sensor and pull the calibration data
	barometer.Initialize();
	barometer.PrintCalibrationData();

	publishBMPTimer.initializeMs(TIMER * 3000, publishBMP).start(); // start publish BMP180 sensor data
}

#endif /* INCLUDE_BMP180_H_ */
