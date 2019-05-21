/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 15.07.2015
Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14)
*/
#pragma once

#include "SPIBase.h"
#include "SPISettings.h"

class SPISoft : public SPIBase
{
public:
	SPISoft(uint16_t miso, uint16_t mosi, uint16_t sck, uint8_t delay)
		: mMISO(miso), mMOSI(mosi), mCLK(sck), m_delay(delay)
	{
	}

	/*
	 *  begin(): Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
	 */
	void begin() override; //setup pins

	/*
	 * end(): Disables the SPI bus (leaving pin modes unchanged).
	 */
	void end() override
	{
	}

	/*
	 * beginTransaction(): Initializes the SPI bus using the defined SPISettings.
	 */
	void beginTransaction(SPISettings mySettings) override
	{
	}

	/*
	 * endTransaction(): Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	void endTransaction() override
	{
	}

	/*
	 * transfer(), transfer16()
	 *
	 * SPI transfer is based on a simultaneous send and receive: the received data is returned in receivedVal (or receivedVal16). In case of buffer transfers the received data is stored in the buffer in-place (the old data is replaced with the data received).
	 *
	 * 		receivedVal = SPI.transfer(val)
	 * 		receivedVal16 = SPI.transfer16(val16)
	 * 		SPI.transfer(buffer, size)
	 */
	void transfer(uint8_t* buffer, size_t size) override;

	unsigned char transfer(unsigned char val) override
	{
		transfer(&val, 1);
		return val;
	}

	unsigned short transfer16(unsigned short val) override
	{
		transfer(reinterpret_cast<uint8_t*>(&val), 2);
		return val;
	}

	/**
	 * @brief Set microsecond delay for the SCK signal. Impacts SPI speed
	*/
	void setDelay(uint8_t dly)
	{
		m_delay = dly;
	}

private:
	SPISettings mSPISettings;
	uint16_t mMISO, mMOSI, mCLK;
	uint8_t m_delay;
};
