#include <SmingCore.h>
#include <Libraries/SI7021/SI7021.h>
#include <math.h>

SI7021 hydrometer;
Timer procTimer_ht;
Timer procTimer_olt;

#define I2C_SCL 5 // SCL
#define I2C_SDA 4 // SCA

//LN10 - 2.30258509299404568402
double getDewPoint(unsigned int humidity, int temperature)
{
	const double C = 235.66; // Constant from the datasheet HTU/SHT
	return 1 / (1 / ((double)temperature / 100 + C) + (2 - log((double)humidity) / 2.30258509299404568402) / 1762.39) -
		   C;
}

void si_read_ht()
{
	if(!hydrometer.begin())
		Serial.println("Could not connect to SI7021.");
	Serial.print("Start reading Humidity and Temperature");
	Serial.println(); // Start a new line.

	si7021_env env_data = hydrometer.getHumidityAndTemperature();

	if(env_data.error_crc == 1) {
		Serial.print("\tCRC ERROR: ");
		Serial.println(); // Start a new line.
	} else {
		// Print out the Temperature
		Serial.print("\tTemperature: ");
		float tprint = env_data.temperature;
		Serial.print(tprint / 100);
		Serial.print("C");
		Serial.println(); // Start a new line.
		// Print out the Humidity Percent
		Serial.print("\tHumidity: ");
		Serial.print(env_data.humidityPercent);
		Serial.print("%");
		Serial.println(); // Start a new line.
		// Print out the Dew Point
		Serial.print("\tDew Point: ");
		Serial.print(getDewPoint(env_data.humidityPercent, env_data.temperature));
		Serial.print("C");
		Serial.println();
	}
}

void si_read_olt()
{
	if(!hydrometer.begin())
		Serial.println("Could not connect to SI7021.");
	Serial.print("Start reading Temperature");
	Serial.println(); // Start a new line.

	si7021_olt olt_data = hydrometer.getTemperatureOlt();

	if(olt_data.error_crc == 1) {
		Serial.print("\tCRC ERROR: ");
		Serial.println(); // Start a new line.
	} else {
		// Print out the Temperature
		Serial.print("\tTemperature: ");
		float tprint = olt_data.temperature;
		Serial.print(tprint / 100);
		Serial.print("C");
		Serial.println(); // Start a new line.
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial
	Serial.print("Start I2c");
	Wire.pins(I2C_SDA, I2C_SCL); // SDA, SCL
	Wire.begin();
	procTimer_ht.initializeMs(10000, si_read_ht).start();
	procTimer_olt.initializeMs(15000, si_read_olt).start();
}
