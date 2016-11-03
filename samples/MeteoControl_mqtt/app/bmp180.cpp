#ifndef INCLUDE_BMP180_H_
#define INCLUDE_BMP180_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/BMP180/BMP180.h>

#include "../include/configuration.h"

//    GPIO0 SCL
//    GPIO2 SDA

BMP180 barometer;

Timer publishBMPTimer;

void publishBMP()
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.println("\n*********************************************");
	Serial.print("\rStart reading BMP180 sensor");
	if(!barometer.EnsureConnected()) {
		Serial.println("Could not connect to BMP180");
	}
	else {
	// Retrive the current pressure in Pascals
	long currentPressure = barometer.GetPressure();
	// Давление в мм р.с.
	float BMPPress = currentPressure / 133.322;

	// Print out the Pressure
	Serial.print("\nPressure: ");
	Serial.print(BMPPress);
	Serial.print(" мм р.с.");
	mqtt.publish(BMP_P, String(BMPPress));

	// Retrive the current temperature in degrees celcius
	float BMPTemp = barometer.GetTemperature();

	// Print out the Temperature
	Serial.print("\nTemperature: ");
	Serial.print(BMPTemp);
//	Serial.write(176);
	Serial.print(" *C");
	mqtt.publish(BMP_T, String(BMPTemp));
	Serial.println("\nBMP180 sensor read and transmitted to server");
	Serial.println("\r*********************************************");
	}
}

void BMPinit()
{
	// When we have connected, we reset the device to ensure a clean start.
	barometer.SoftReset();

	// Now we initialize the sensor and pull the calibration data
    	barometer.Initialize();
	barometer.PrintCalibrationData();

	//wait for sensor startup
//	delay(1000);

	publishBMPTimer.initializeMs(TIMER * 3000, publishBMP).start();	// start publish BMP180 sensor data
}

#endif /* INCLUDE_BMP180_H_ */
