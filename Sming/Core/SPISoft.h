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

// for compatibility when porting from Arduino
#define SPI_HAS_TRANSACTION 1

class SPISoft : public SPIBase
{
public:
	SPISoft(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t delay)
		: mMISO(miso), mMOSI(mosi), mCLK(sck), m_delay(delay)
	{
	}

	bool begin() override;

	void end() override
	{
	}

	void endTransaction() override;

	using SPIBase::transfer;
	uint32_t transfer32(uint32_t val, uint8_t bits = 32) override;
	void transfer(uint8_t* buffer, size_t size) override;

	/**
	 * @brief Set microsecond delay for the SCK signal. Impacts SPI speed
	*/
	void setDelay(uint8_t dly)
	{
		m_delay = dly;
	}

protected:
	void prepare(SPISettings& settings) override;

private:
	uint32_t transferWordLSB(uint32_t word, uint8_t bits);
	uint32_t transferWordMSB(uint32_t word, uint8_t bits);

	uint8_t mMISO;
	uint8_t mMOSI;
	uint8_t mCLK;
	uint8_t m_delay;
	SpiMode dataMode{SPI_MODE0};
	uint8_t cpol{0};
	uint8_t cksample{0};
	bool lsbFirst{false};
};
