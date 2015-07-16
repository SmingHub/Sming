#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/DHT/DHT.h>

#define WORK_PIN 0 // GPIO0

DHT dht(WORK_PIN);

void displayComfort();

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	dht.begin();

	/*first reading method (Adafruit compatible) */

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
		Serial.println(" *C\n");
	}


	/* improved reading method */
	TempAndHumidity th;
	if(dht.readTempAndHumidity(th))
	{
		Serial.print("Humidity: ");
		Serial.print(h);
		Serial.print(" %\t");
		Serial.print("Temperature: ");
		Serial.print(t);
		Serial.println(" *C\n");
	}
	else
	{
		Serial.print("Failed to read from DHT: ");
		Serial.print(dht.getLastError());
	}

	/* other goodies */

	/*
	 * Heatindex is the percieved temperature taking humidity into account
	 * More: https://en.wikipedia.org/wiki/Heat_index
	 * */
	Serial.printf("Heatindex: %d *C\n", dht.getHeatIndex());

	/*
	 * Dewpoint is the temperature where condensation starts.
	 * Water vapors will start condensing on an object having this temperature or below.
	 * More: https://en.wikipedia.org/wiki/Dew_point
	 * */
	Serial.printf("Dewpoint: %d *C\n", dht.getDewPoint(DEW_ACCURATE_FAST));

	/*
	 * Determine thermal comfort according to http://epb.apogee.net/res/refcomf.asp
	 * */
	displayComfort();

}

void displayComfort()
{
	ComfortState cf;

	Serial.print("Comfort is at ");
	Serial.print(dht.getComfortRatio(cf));
	Serial.print(" percent, which is ");

	switch(cf)
	{
	case Comfort_OK:
		Serial.print("OK");
		break;
	case Comfort_TooHot:
		Serial.print("Too Hot");
		break;
	case Comfort_TooCold:
		Serial.print("Too Cold");
		break;
	case Comfort_TooDry:
		Serial.print("Too Dry");
		break;
	case Comfort_TooHumid:
		Serial.print("Too Humid");
		break;
	case Comfort_HotAndHumid:
		Serial.print("Hot And Humid");
		break;
	case Comfort_HotAndDry:
		Serial.print("Hot And Dry");
		break;
	case Comfort_ColdAndHumid:
		Serial.print("Cold And Humid");
		break;
	case Comfort_ColdAndDry:
		Serial.print("Cold And Dry");
		break;
	default:
		Serial.print("Unknown:");
		Serial.print(cf);
		break;
	}

	Serial.print("\n");
}

