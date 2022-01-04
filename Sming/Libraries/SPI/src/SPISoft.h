/*
Author: ADiea
Project: Sming for ESP8266
License: MIT
Date: 15.07.2015
Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14)
*/
#pragma once

#include "SPIBase.h"

/**
 * @brief Software-based SPI master
 *
 * Intended for ESP8266 due to limited I/O but will work on any architecture.
 * 
 * @addtogroup soft_spi
 */
class SPISoft : public SPIBase
{
public:
	/**
	 * @brief Use same pins as hardware SPI by default
	 */
	SPISoft()
	{
	}

	SPISoft(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t delay = 0) : pins{sck, miso, mosi}, m_delay(delay)
	{
	}

	SPISoft(const SpiPins& pins, uint8_t delay = 0) : pins(pins), m_delay(delay)
	{
	}

	SPISoft(uint8_t delay) : m_delay(delay)
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
	 * @brief Set delay factor for the SCK signal. Impacts SPI speed.
	 *
	 * Requires code to be compiled with ENABLE_SPISOFT_DELAY=1.
	 *
	 * ESP8266 only: The delay will be automatically calculated for a requested
	 * clock speed when `begin()` or `beginTransaction()` are called.
	 * To use only the manually programmed delay, set the clock speed to zero.
	 */
	void setDelay(uint8_t delay)
	{
		m_delay = delay;
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
