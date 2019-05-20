#ifndef INCLUDE_SI7021_H_
#define INCLUDE_SI7021_H_

#include <SmingCore.h>
#include <Libraries/SI7021/SI7021.h>

#include "configuration.h"

SI7021 hydrometer;

Timer publishSITimer;

void publishSI()
{
	if(mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect

	Serial.print("*********************************************");
	Serial.println("Start reading SI7021 sensor");

	if(!hydrometer.begin()) {
		Serial.println("Could not connect to SI7021");
	} else {
		si7021_env data = hydrometer.getHumidityAndTemperature();
		if(data.error_crc == 1) {
			Serial.println("\tCRC ERROR");
		} else {
			float SIhum = data.humidityPercent;
			// Print out the humidity
			Serial.print("Humidity: ");
			Serial.print(SIhum);
			Serial.println("%");
			mqtt.publish(SI_H, String(SIhum));
			float SITemp = data.temperature;
			// Print out the Temperature
			Serial.print("Temperature: ");
			Serial.print(SITemp / 100);
			Serial.println(" *C");
			mqtt.publish(SI_T, String(SITemp / 100));
			Serial.println("SI sensor read and transmitted to server");
			Serial.println("*********************************************");
		}
	}
}

void SIinit()
{
	publishSITimer.initializeMs(TIMER * 1000, publishSI).start(); // start publish SI sensor data
}

#endif /* INCLUDE_SI7021_H_ */
