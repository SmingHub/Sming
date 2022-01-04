/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 15.07.2015
Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14)
*/
#pragma once

#include "SPIBase.h"

class SPISoft : public SPIBase
{
public:
	/**
	 * @brief Use same pins as hardware SPI by default
	 */
	SPISoft()
	{
	}

	SPISoft(uint8_t delay) : m_delay(delay)
	{
	}

	SPISoft(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t delay) : pins{sck, miso, mosi}, m_delay(delay)
	{
	}

	SPISoft(const SpiPins& pins, uint8_t delay) : pins(pins), m_delay(delay)
	{
	}

	bool setup(SpiPins pins)
	{
		this->pins = pins;
		return true;
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

	bool loopback(bool enable) override;

protected:
	void prepare(SPISettings& settings) override;

private:
	uint32_t transferWordLSB(uint32_t word, uint8_t bits);
	uint32_t transferWordMSB(uint32_t word, uint8_t bits);

	SpiPins pins;
	uint8_t m_delay{0};
	SpiMode dataMode{SPI_MODE0};
	uint8_t cpol{0};
	uint8_t cksample{0};
	bool lsbFirst{false};
};
