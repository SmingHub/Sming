#include "spi.h"
#include "SparkFunAutoDriver.h"

// Constructors
AutoDriver::AutoDriver(int CSPin, int resetPin, int busyPin)
{
	_CSPin = CSPin;
	_resetPin = resetPin;
	_busyPin = busyPin;

	//zhivko: commented out SPIConfig arduino code
	//SPIConfig();
	//metalphreak spi driver descriebed in: http://www.esp8266.com/viewtopic.php?f=13&t=1467
	spi_init((uint8)HSPI);
}

AutoDriver::AutoDriver(int CSPin, int resetPin)
{
	_CSPin = CSPin;
	_resetPin = resetPin;
	_busyPin = -1;

	//zhivko: commented out SPIConfig arduino code
	//SPIConfig();
	spi_init((uint8)HSPI);
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
