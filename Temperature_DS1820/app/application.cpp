#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <Libraries/DS18S20/ds18s20.h>

DS18S20 ReadTemp;
Timer procTimer;

void readData()
{
	if (ReadTemp.IsValidTemperature())
	{

		Serial.println("**********************************");
	    Serial.print("Demo reding temperature = ");
		Serial.print(ReadTemp.GetCelsius());
		Serial.print(" Celsius, (");
		Serial.print(ReadTemp.GetFahrenheit());
		Serial.println(" Fahrenheit)");
		Serial.println("**********************************");
	}
	else
		Serial.println("No valid Measure so far!");

	ReadTemp.StartMeasure();  // next measure
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

        ReadTemp.Init(2);  // select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure();

	procTimer.initializeMs(10000, readData).start();   // every 10 seconds
}

