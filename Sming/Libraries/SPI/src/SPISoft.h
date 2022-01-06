/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPISoft.h
 *
 * Author: ADiea
 * Project: Sming for ESP8266
 * License: MIT
 * Date: 15.07.2015
 * Descr: Implement software SPI for HW configs other than hardware SPI pins(GPIO 12,13,14)
 * 
 * @author mikee47 January 2022
 *
 * Rewritten to implement bit ordering, modes, delays and transactions.
 *
 ****/

#pragma once

#include "SPIBase.h"

/**
 * @defgroup soft_spi SPI software implementation
 */

/**
 * @brief Software-based SPI master
 *
 * Intended for ESP8266 due to limited I/O but will work on any architecture.
 * 
 * @ingroup soft_spi
 */
class SPISoft : public SPIBase
{
public:
	/**
	 * @name Constructors
	 * @{
	 */

	/**
	 * @brief Default constructor uses same pins as hardware SPI
	 */
	SPISoft();

	/**
	 * @brief Specify pins to use plus optional delay
	 *
	 * Delay is ignored if code is not compiled with SPISOFT_DELAY < 0.
	 */
	SPISoft(uint8_t miso, uint8_t mosi, uint8_t sck, uint8_t delay = 0) : SPISoft({sck, miso, mosi}, delay)
	{
	}

	/**
	 * @brief Specify pins plus optional delay
	 */
	SPISoft(const SpiPins& pins, uint8_t delay = 0) : SPIBase(pins), m_delay(delay)
	{
	}

	/**
	 * @brief Use default pins but provide a delay
	 */
	SPISoft(uint8_t delay);

	/** @} */

	bool setup(SpiPins pins)
	{
		this->mPins = pins;
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
	 * Requires code to be compiled with SPISOFT_DELAY < 0.
	 *
	 * ESP8266 only: The delay will be automatically calculated for a requested
	 * clock speed when `begin()` or `beginTransaction()` are called.
	 * To use only the manually programmed delay, set the clock speed to zero.
	 */
	void setDelay(uint8_t delay)
	{
		m_delay = delay;
	}

	bool loopback(bool enable) override
	{
		(void)enable;
		return false;
	}

protected:
	void prepare(SPISettings& settings) override;

private:
	uint8_t transferByteLSB(uint8_t word);
	uint8_t transferByteMSB(uint8_t word);
	uint32_t transferWordLSB(uint32_t word, uint8_t bits);
	uint32_t transferWordMSB(uint32_t word, uint8_t bits);

	uint8_t m_delay{0};
	SpiMode dataMode{SPI_MODE0};
	uint8_t cpol{0};
	uint8_t cksample{0};
	bool lsbFirst{false};
};
