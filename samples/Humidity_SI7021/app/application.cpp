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
	if(!hydrometer.begin()) {
		Serial.println(_F("Could not connect to SI7021."));
	}
	Serial.println(_F("Start reading Humidity and Temperature"));

	si7021_env env_data = hydrometer.getHumidityAndTemperature();

	if(env_data.error_crc == 1) {
		Serial.println(_F("\tCRC ERROR: "));
	} else {
		Serial << _F("\tTemperature: ") << env_data.temperature / 100.0 << " °C" << endl;
		Serial << _F("\tHumidity: ") << env_data.humidityPercent << '%' << endl;
		Serial << _F("\tDew Point: ") << getDewPoint(env_data.humidityPercent, env_data.temperature) << " °C" << endl;
	}
}

void si_read_olt()
{
	if(!hydrometer.begin()) {
		Serial.println(_F("Could not connect to SI7021."));
	}
	Serial.println(_F("Start reading Temperature"));

	si7021_olt olt_data = hydrometer.getTemperatureOlt();

	if(olt_data.error_crc == 1) {
		Serial.println(_F("\tCRC ERROR: "));
	} else {
		Serial << _F("\tTemperature: ") << olt_data.temperature / 100.0 << " °C" << endl;
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial
	Serial.print(_F("Start I2c"));
	Wire.pins(I2C_SDA, I2C_SCL); // SDA, SCL
	Wire.begin();
	procTimer_ht.initializeMs(10000, si_read_ht).start();
	procTimer_olt.initializeMs(15000, si_read_olt).start();
}
