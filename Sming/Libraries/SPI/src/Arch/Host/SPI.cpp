#include "SPI.h"

SPIClass SPI;

bool SPIClass::begin()
{
	return false;
}

void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
}

void SPIClass::prepare(SPISettings& settings)
{
}

bool SPIClass::loopback(bool enable)
{
	(void)enable;
	return true;
}
