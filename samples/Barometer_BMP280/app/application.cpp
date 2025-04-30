/***************************************************************************
 * Sming port of example from Adafruit_BMP280_Library

  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <SmingCore.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

#define SEALEVELPRESSURE_HPA 1013.25

namespace
{
Adafruit_BMP280 bmp;
SimpleTimer procTimer;

// Will use default pins for selected architecture. You can override values here
// #define SDA 2
// #define SCL 0

void printValues()
{
	Serial << _F("Temperature = ") << bmp.readTemperature() << " °C" << endl;
	Serial << _F("Pressure = ") << bmp.readPressure() / 100.0F << " hPa" << endl;
	Serial << _F("Approx. Altitude = ") << bmp.readAltitude(SEALEVELPRESSURE_HPA) << " m" << endl;
	Serial.println();
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable/disable debug output

	Serial.println(_F("BMP280 test"));

#ifdef SDA
	Wire.pins(SDA, SCL);
#endif

	if(!bmp.begin()) { // if(!bme.begin(0x76, &Wire)) { if you need a specific address
		Serial.println(_F("Could not find a valid BMP280 sensor, check wiring, address, sensor ID!"));
		Serial << _F("SensorID was: 0x") << String(bmp.sensorID(), HEX) << endl;
		Serial << _F("  ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\r\n"
					 "  ID of 0x56-0x58 represents a BMP 280,\r\n"
					 "  ID of 0x60 represents a BME 280.\r\n"
					 "  ID of 0x61 represents a BME 680.\r\n");
		return;
	}

	/* Default settings from datasheet */
	bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,	 // Operating Mode
					Adafruit_BMP280::SAMPLING_X2,	 // Temp. oversampling
					Adafruit_BMP280::SAMPLING_X16,	// Pressure oversampling
					Adafruit_BMP280::FILTER_X16,	  // Filtering
					Adafruit_BMP280::STANDBY_MS_500); // Standby time

	Serial.println(_F("-- Default Test --"));

	Serial.println();

	procTimer.initializeMs<3000>(printValues).start();
}
