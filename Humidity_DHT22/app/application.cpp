#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DHT/DHT.h>

#define WORK_PIN 0 // GPIO0

DHT dht(WORK_PIN, DHT22);

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 230400 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	dht.begin();

	float h = dht.readHumidity();
	float t = dht.readTemperature();

	// check if returns are valid, if they are NaN (not a number) then something went wrong!
	if (isnan(t) || isnan(h))
	{
		Serial.println("Failed to read from DHT");
	} else {
		Serial.print("Humidity: ");
		Serial.print(h);
		Serial.print(" %\t");
		Serial.print("Temperature: ");
		Serial.print(t);
		Serial.println(" *C");
	}
}
