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

	Serial.println(_F("\r\n"
					  "DHT improved lib"));
	Serial << _F("TickCount=") << RTC.getRtcNanoseconds() / 1000000
		   << _F("; Need to wait 1 second for the sensor to boot up") << endl;
}

void onTimer_readTemperatures()
{
	//* try different reading methods (Adafruit compatible) vs improved */
	static bool toggle = false;
	toggle = !toggle;
	float humidity = 0;
	float temperature = 0;
	Serial << _F("TickCount=") << RTC.getRtcNanoseconds() / 1000000 << endl;
	if(toggle) {
		Serial.println(_F("Read using Adafruit API methods"));
		humidity = dht.getHumidity();
		temperature = dht.getTemperature();

		// check if returns are valid, if they are NaN (not a number) then something went wrong!
		if(dht.getStatus() == DHTesp::ERROR_NONE) {
			Serial << _F("\tHumidity: ") << humidity << _F("% Temperature: ") << temperature << " °C" << endl;
		} else {
			Serial << _F("Failed to read from DHT: ") << dht.getStatus() << endl;
		}
	} else {
		//* improved reading method
		Serial.println(_F("\r\n"
						  "Read using new API methods"));
		TempAndHumidity th = dht.getTempAndHumidity();
		humidity = th.humidity;
		temperature = th.temperature;

		if(dht.getStatus() == DHTesp::ERROR_NONE) {
			Serial << _F("\tHumidity: ") << th.humidity << _F("% Temperature: ") << th.temperature << " °C" << endl;
		} else {
			Serial << _F("Failed to read from DHT: ") << dht.getStatus() << endl;
		}
	}

	//  Other goodies:
	//
	//  Heatindex is the perceived temperature taking humidity into account
	//  More: https://en.wikipedia.org/wiki/Heat_index
	//
	Serial << _F("Heatindex: ") << dht.computeHeatIndex(temperature, humidity) << " °C" << endl;

	//
	//  Dewpoint is the temperature where condensation starts.
	//  Water vapors will start condensing on an object having this temperature or below.
	//  More: https://en.wikipedia.org/wiki/Dew_point
	//
	Serial << _F("Dewpoint: ") << dht.computeDewPoint(temperature, humidity) << " °C" << endl;

	//
	// Determine thermal comfort according to http://epb.apogee.net/res/refcomf.asp
	//
	ComfortState cf;

	Serial << _F("Comfort is at ") << dht.getComfortRatio(cf, temperature, humidity) << " %, (";

	switch(cf) {
	case Comfort_OK:
		Serial.print(_F("OK"));
		break;
	case Comfort_TooHot:
		Serial.print(_F("Too Hot"));
		break;
	case Comfort_TooCold:
		Serial.print(_F("Too Cold"));
		break;
	case Comfort_TooDry:
		Serial.print(_F("Too Dry"));
		break;
	case Comfort_TooHumid:
		Serial.print(_F("Too Humid"));
		break;
	case Comfort_HotAndHumid:
		Serial.print(_F("Hot And Humid"));
		break;
	case Comfort_HotAndDry:
		Serial.print(_F("Hot And Dry"));
		break;
	case Comfort_ColdAndHumid:
		Serial.print(_F("Cold And Humid"));
		break;
	case Comfort_ColdAndDry:
		Serial.print(_F("Cold And Dry"));
		break;
	default:
		Serial.print(_F("Unknown:"));
		Serial.print(cf);
		break;
	}

	Serial.println(')');
}
