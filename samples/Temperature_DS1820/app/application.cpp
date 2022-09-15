#include <SmingCore.h>
#include <Libraries/DS18S20/ds18s20.h>

#define I2C_PIN 4

DS18S20 ReadTemp;
Timer procTimer;

//**********************************************************
// DS18S20 example, reading
// You can connect multiple sensors to a single port
// (At the moment 4 pcs - it depends on the definition in the library)
// Measuring time: 1.2 seconds * number of sensors
// The main difference with the previous version of the demo:
//  - Do not use the Delay function () which is discouraged by the manufacturer of ESP8266
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
		auto sensorCount = ReadTemp.GetSensorsCount();
		if(sensorCount > 0) {
			Serial.println(_F("******************************************"));
		}
		Serial.println(_F(" Reading temperature DEMO"));
		// prints for all sensors
		for(unsigned a = 0; a < sensorCount; a++) {
			Serial << " T" << a + 1 << " = ";
			if(ReadTemp.IsValidTemperature(a)) {
				Serial << ReadTemp.GetCelsius(a) << _F(" Celsius, (") << ReadTemp.GetFahrenheit(a) << _F(" Fahrenheit)")
					   << endl;
			} else {
				Serial.println(_F("Temperature not valid"));
			}

			Serial << _F(" <Sensor id.") << String(ReadTemp.GetSensorID(a), HEX, 32) << '>' << endl;
		}
		Serial.println(_F("******************************************"));
		ReadTemp.StartMeasure(); // next measure, result after 1.2 seconds * number of sensors
	} else
		Serial.println(_F("No valid Measure so far! wait please"));
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	ReadTemp.Init(I2C_PIN);
	ReadTemp.StartMeasure(); // first measure start,result after 1.2 seconds * number of sensors

	procTimer.initializeMs<10000>(readData).start();
}
