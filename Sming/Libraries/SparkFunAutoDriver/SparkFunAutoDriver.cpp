#include "SparkFunAutoDriver.h"


// Constructors
AutoDriver::AutoDriver(int CSPin, int resetPin, int busyPin)
{
	_CSPin = CSPin;
	_resetPin = resetPin;
	_busyPin = busyPin;

	pinMode(_resetPin, OUTPUT);
	pinMode(_CSPin, OUTPUT);
	pinMode(_busyPin, INPUT);

	if(_resetPin>0)
	{
		digitalWrite(_resetPin, LOW);
		delayMicroseconds(50);
		digitalWrite(_resetPin, HIGH);
	}
	digitalWrite(_CSPin, LOW);

	//zhivko: commented out SPIConfig arduino code
	//SPIConfig();
	//metalphreak spi driver described in: http://www.esp8266.com/viewtopic.php?f=13&t=1467
	hwSpi = HwSPIClass();
	hwSpi.begin(HSPI);


}

AutoDriver::AutoDriver(int CSPin, int resetPin)
{
	_CSPin = CSPin;
	_resetPin = resetPin;
	_busyPin = -1;

	pinMode(_resetPin, OUTPUT);
	pinMode(_CSPin, OUTPUT);

	if(_resetPin>0)
	{
		digitalWrite(_resetPin, LOW);
		delayMicroseconds(50);
		digitalWrite(_resetPin, HIGH);
	}
	digitalWrite(_CSPin, LOW);

	//zhivko: commented out SPIConfig arduino code
	//SPIConfig();
	hwSpi = HwSPIClass();
	hwSpi.begin(HSPI);

}

int AutoDriver::busyCheck(void)
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


