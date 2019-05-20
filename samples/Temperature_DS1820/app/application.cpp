#include <SmingCore.h>
#include <Libraries/OneWire/OneWire.h>

#include <Libraries/DS18S20/ds18s20.h>

DS18S20 ReadTemp;
Timer procTimer;
//**********************************************************
// DS18S20 example, reading
// You can connect multiple sensors to a single port
// (At the moment 4 pcs - it depends on the definition in the library)
// Measuring time: 1.2 seconds * number of sensors
// The main difference with the previous version of the demo:
//  - Do not use the Delay function () which discourages by manufacturer of ESP8266
//  - We can read several sensors
// Usage:
//  Call Init to setup pin eg. ReadTemp.Init(2);   //pin 2 selected
//  Call ReadTemp.StartMeasure();
//   if ReadTemp.MeasureStatus() false read sensors
//   You can recognize sensors by the ID or index.
//***********************************************************
void readData()
{
	if(!ReadTemp.MeasureStatus()) // the last measurement completed
	{
		if(ReadTemp.GetSensorsCount()) // is minimum 1 sensor detected ?
			Serial.println("******************************************");
		Serial.println(" Reding temperature DEMO");
		for(uint8_t a = 0; a < ReadTemp.GetSensorsCount(); a++) // prints for all sensors
		{
			Serial.print(" T");
			Serial.print(a + 1);
			Serial.print(" = ");
			if(ReadTemp.IsValidTemperature(a)) // temperature read correctly ?
			{
				Serial.print(ReadTemp.GetCelsius(a));
				Serial.print(" Celsius, (");
				Serial.print(ReadTemp.GetFahrenheit(a));
				Serial.println(" Fahrenheit)");
			} else
				Serial.println("Temperature not valid");

			Serial.print(" <Sensor id.");

			uint64_t info = ReadTemp.GetSensorID(a) >> 32;
			Serial.print((uint32_t)info, 16);
			Serial.print((uint32_t)ReadTemp.GetSensorID(a), 16);
			Serial.println(">");
		}
		Serial.println("******************************************");
		ReadTemp.StartMeasure(); // next measure, result after 1.2 seconds * number of sensors
	} else
		Serial.println("No valid Measure so far! wait please");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	ReadTemp.Init(4);		 // select PIN It's required for one-wire initialization!
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors

	procTimer.initializeMs(10000, readData).start(); // every 10 seconds
}
