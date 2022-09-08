/***************************************************************************
 * Sming port of example from Adafruit_BME280_Library

  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <SmingCore.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
Timer procTimer;

// Will use default pins for selected architecture. You can override values here
// #define SDA 4
// #define SCL 5

void printValues()
{
	Serial << _F("Temperature = ") << bme.readTemperature() << " °C" << endl;
	Serial << _F("Pressure = ") << bme.readPressure() / 100.0F << " hPa" << endl;
	Serial << _F("Approx. Altitude = ") << bme.readAltitude(SEALEVELPRESSURE_HPA) << " m" << endl;
	Serial << _F("Humidity = ") << bme.readHumidity() << " %" << endl;
	Serial.println();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable/disable debug output

	Serial.println(_F("BME280 test"));

#ifdef SDA
	Wire.pins(SDA, SCL);
#endif

	if(!bme.begin()) { // if(!bme.begin(0x76, &Wire)) { if you need a specific address
		Serial.println(_F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
		Serial << _F("SensorID was: 0x") << String(bme.sensorID(), HEX) << endl;
		Serial << _F("  ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\r\n"
					 "  ID of 0x56-0x58 represents a BMP 280,\r\n"
					 "  ID of 0x60 represents a BME 280.\r\n"
					 "  ID of 0x61 represents a BME 680.\r\n");
		return;
	}

	Serial.println(_F("-- Default Test --"));

	Serial.println();

	procTimer.initializeMs<3000>(printValues).start();
}
