#include <SmingCore.h>
#include <Libraries/DHTesp/DHTesp.h>

//#define WORK_PIN 14 // GPIO14
#define WORK_PIN 2

DHTesp dht;

Timer readTemperatureProcTimer;
void onTimer_readTemperatures();

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	dht.setup(WORK_PIN, DHTesp::DHT22);
	readTemperatureProcTimer.initializeMs(5 * 1000, onTimer_readTemperatures).start(); // every so often.

	Serial.println("\nDHT improved lib");
	Serial.print("TickCount=");
	Serial.print((int)(RTC.getRtcNanoseconds() / 1000000));
	Serial.println("Need to wait 1 second for the sensor to boot up");
}

void onTimer_readTemperatures()
{
	//* try different reading methods (Adafruit compatible) vs improved */
	static bool toggle = false;
	toggle = !toggle;
	float humidity = 0;
	float temperature = 0;
	Serial.print("TickCount=");
	Serial.print((int)(RTC.getRtcNanoseconds() / 1000000));
	if(toggle) {
		Serial.print("Read using Adafruit API methods\n");
		humidity = dht.getHumidity();
		temperature = dht.getTemperature();

		// check if returns are valid, if they are NaN (not a number) then something went wrong!
		if(dht.getStatus() == DHTesp::ERROR_NONE) {
			Serial.print("\tHumidity: ");
			Serial.print(humidity);
			Serial.print("% Temperature: ");
			Serial.print(temperature);
			Serial.print(" *C\n");
		} else {
			Serial.print("Failed to read from DHT: ");
			Serial.println((int)dht.getStatus());
		}
	} else {
		//* improved reading method
		Serial.print("\nRead using new API methods\n");
		TempAndHumidity th;
		th = dht.getTempAndHumidity();
		humidity = th.humidity;
		temperature = th.temperature;

		if(dht.getStatus() == DHTesp::ERROR_NONE) {
			Serial.print("\tHumidity: ");
			Serial.print(th.humidity);
			Serial.print("% Temperature: ");
			Serial.print(th.temperature);
			Serial.print(" *C\n");
		} else {
			Serial.print("Failed to read from DHT: ");
			Serial.print(dht.getStatus());
		}
	}

	//  Other goodies:
	//
	//  Heatindex is the percieved temperature taking humidity into account
	//  More: https://en.wikipedia.org/wiki/Heat_index
	//
	Serial.print("Heatindex: ");
	Serial.print(dht.computeHeatIndex(temperature, humidity));
	Serial.print("*C\n");

	//
	//  Dewpoint is the temperature where condensation starts.
	//  Water vapors will start condensing on an object having this temperature or below.
	//  More: https://en.wikipedia.org/wiki/Dew_point
	//
	Serial.printf("Dewpoint: ");
	Serial.print(dht.computeDewPoint(temperature, humidity));
	Serial.print("*C\n");

	//
	// Determine thermal comfort according to http://epb.apogee.net/res/refcomf.asp
	//
	ComfortState cf;

	Serial.print("Comfort is at ");
	Serial.print(dht.getComfortRatio(cf, temperature, humidity));
	Serial.print(" percent, (");

	switch(cf) {
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

	Serial.print(")\n");
}
