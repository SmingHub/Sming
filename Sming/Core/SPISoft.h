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

	bool begin() override;

	void end() override
	{
	}

	void transfer(uint8_t* buffer, size_t size) override;

	/**
	 * @brief Set microsecond delay for the SCK signal. Impacts SPI speed
	*/
	void setDelay(uint8_t dly)
	{
		m_delay = dly;
	}

protected:
	void prepare(SPISettings& settings) override
	{
	}

private:
	uint16_t mMISO, mMOSI, mCLK;
	uint8_t m_delay;
};
