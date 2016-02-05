/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/Wire.h"

#include "../SmingCore/Digital.h"
#include "../Wiring/WiringFrameworkIncludes.h"

TwoWire::TwoWire(int pinSCL, int pinSDA)
{
	SCL = pinSCL;
	SDA = pinSDA;
	targetAddress = -1;
	txLen = 0;
	rxPos = 0;
	rxLen = 0;
	master = NULL;
}

TwoWire::~TwoWire()
{
	if (master != NULL)
		delete master;
}

void TwoWire::pins(int pinSCL, int pinSDA)
{
	SCL = pinSCL;
	SDA = pinSDA;
}

void TwoWire::begin()
{
	if (master != NULL) return;
	master = new SoftI2cMaster(SDA, SCL);
	master->stop(); // Ready to work
}

void TwoWire::beginTransmission(uint8_t address)
{
	if (targetAddress != -1)
		endTransmission();

	targetAddress = (uint8_t)(address << 1);
	txLen = 0;
	rxPos = 0;
	rxLen = 0;
}

uint8_t TwoWire::endTransmission(bool sendStop /*= true*/)
{
	if (targetAddress == -1) return 4; // other error

	uint8_t result = 0;
	result = pushData();

	if (sendStop)
		master->stop();
	else
	{
		// Restart mode not tested!!!
		digitalWrite(SDA, HIGH);
		digitalWrite(SCL, HIGH);
	}
	return result;
}

uint8_t TwoWire::pushData()
{
	if (txLen == -1) return 1; // data too long to fit in transmit buffer

	if (!master->start(targetAddress | I2C_WRITE)) return 2; // received NACK on transmit of address

	for (int i = 0; i < txLen; i++)
		if (!master->write(txBuf[i]))
			return 3; // received NACK on transmit of data

	targetAddress = -1;
	txLen = 0;
	return 0;
}

uint8_t TwoWire::requestFrom(int address, int quantity, bool sendStop /* = true*/)
{
	rxPos = 0;
	rxLen = 0;

	if (!master->start(((uint8_t)(address << 1)) | I2C_READ)) return 0; // received NACK on transmit of address

	for (int i = 0; i < quantity; i++)
		rxBuf[rxLen++] = master->read(quantity == i + 1);

	if(sendStop)
		master->stop();

	return quantity;
}

size_t TwoWire::write(uint8_t data)
{
    if(txLen >= BUFFER_LENGTH || txLen == -1)
    {
      txLen = -1; // Overflow :(
      return 0;
    }

    txBuf[txLen++] = data;
    return 1;
}

int TwoWire::available()
{
	return rxLen - rxPos;
}

int TwoWire::read()
{
	if (rxLen > rxPos)
		return rxBuf[rxPos++];
	else
		return -1;
}

int TwoWire::peek()
{
	return rxBuf[rxPos];
}

void TwoWire::flush()
{
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
	for(size_t i = 0; i < quantity; i++)
		write(data[i]);

	return quantity;
}

TwoWire Wire = TwoWire(I2C_DEFAULT_SCL_PIN, I2C_DEFAULT_SDA_PIN);
