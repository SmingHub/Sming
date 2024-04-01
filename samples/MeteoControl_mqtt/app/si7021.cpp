#include <SmingCore.h>
#include <Libraries/SI7021/SI7021.h>

#include "configuration.h"

namespace
{
SI7021 hydrometer;
SimpleTimer publishSITimer;

void publishSI()
{
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient(); // Auto reconnect
	}

	Serial.println(_F("*********************************************\r\n"
					  "Start reading SI7021 sensor"));

	if(!hydrometer.begin()) {
		Serial.println(_F("Could not connect to SI7021"));
		return;
	}

	si7021_env data = hydrometer.getHumidityAndTemperature();
	if(data.error_crc == 1) {
		Serial.println(_F("\tCRC ERROR"));
		return;
	}

	float SIhum = data.humidityPercent;
	// Print out the humidity
	Serial << _F("Humidity: ") << SIhum << '%' << endl;
	mqtt.publish(SI_H, String(SIhum));
	float SITemp = data.temperature;
	// Print out the Temperature
	Serial << _F("Temperature: ") << SITemp / 100 << " °C" << endl;
	mqtt.publish(SI_T, String(SITemp / 100));
	Serial.println(_F("SI sensor read and transmitted to server\r\n"
					  "*********************************************"));
}

} // namespace

void SIinit()
{
	publishSITimer.initializeMs<TIMER * 1000>(publishSI).start(); // start publish SI sensor data
}
