#include "hspi.h"
#include "Sparkfun_L6470.h"
#include "../SmingCore/Digital.h"
/*
 #include "util/delay.h" // Turns out, using the Arduino "delay" function
 //  in a library constructor causes the program to
 //  hang if the constructor is invoked outside of
 //  setup() or hold() (i.e., the user attempts to
 //  create a global of the class.
 */
// Constructors
Sparkfun_L6470::Sparkfun_L6470(uint8_t CSPin, uint8_t resetPin, uint8_t busyPin)
{
	os_printf("Sparkfun_L6470(uint8_t CSPin, uint8_t resetPin, uint8_t busyPin)");
	_CSPin = CSPin;
	_resetPin = resetPin;
	_busyPin = busyPin;

}

Sparkfun_L6470::Sparkfun_L6470(uint8_t CSPin, uint8_t resetPin)
{
	os_printf("Sparkfun_L6470(uint8_t CSPin, uint8_t resetPin)");
	_CSPin = CSPin;
	_resetPin = resetPin;
	_busyPin = 0;

	SPIConfig();
}

void Sparkfun_L6470::SPIConfig()
{
	os_printf("SPI config.\n cs=%d reset=%d\n", _CSPin, _resetPin);
	uint16_t MOSI=13;
	uint16_t MISO=12;
	uint16_t SCK=14;

	pinMode(MOSI, OUTPUT);
	pinMode(MISO, INPUT);
	pinMode(SCK, OUTPUT);
	pinMode(_CSPin, OUTPUT);
	digitalWrite(_CSPin, HIGH);
	pinMode(_resetPin, OUTPUT);
	if (_busyPin != 0)
		pinMode(_busyPin, INPUT_PULLUP);

	// TO-DO
	// how can I properly initialize SPI on esp8266 esp-07 ??
	hspi_start_tx();


	// this is line from arduino example of sparkfun
	//SPISettings settings(5000000, MSBFIRST, SPI_MODE3);

	digitalWrite(_resetPin, LOW);
	delay(5);
	digitalWrite(_resetPin, HIGH);
	delay(5);
}

uint8_t Sparkfun_L6470::busyCheck(void)
{
	if (_busyPin == -1)
	{
		if (getParam(STATUS) & 0x0002)
			return 0;
		else
			return 1;
	}
	else
	{
		if (digitalRead(_busyPin) == HIGH)
			return 0;
		else
			return 1;
	}
}
