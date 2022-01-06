/*
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPIBase.h
 *
 *  Created on: Mar 2, 2016
 *      Author: harry-boe
 *
 */

#pragma once

#include "SPISettings.h"
#include <cstddef>

// for compatibility when porting from Arduino
#define SPI_HAS_TRANSACTION 1

/**
 * @defgroup base_spi SPI support classes
 * @brief    Provides SPI support
 * @{
 */

/**
 * @brief SPI driver uses default pin assignment
 */
static constexpr uint8_t SPI_PIN_DEFAULT{0xff};

/**
 * @brief SPI pin connections
 */
struct SpiPins {
	uint8_t sck{SPI_PIN_DEFAULT};
	uint8_t miso{SPI_PIN_DEFAULT};
	uint8_t mosi{SPI_PIN_DEFAULT};

	bool operator==(const SpiPins& other) const
	{
		return sck == other.sck && miso == other.miso && mosi == other.mosi;
	}
};

/*
 * @brief Base class/interface for SPI implementations
 */
class SPIBase
{
public:
	SPIBase(const SpiPins& pins) : mPins(pins)
	{
	}

	virtual ~SPIBase()
	{
	}

	/**
	 * @brief Initialize the SPI bus by setting SCK and MOSI to outputs, pulling SCK and MOSI low.
	 */
	virtual bool begin() = 0;

	/**
	 * @brief Disable the SPI bus (leaving pin modes unchanged).
	 */
	virtual void end() = 0;

	/**
	 * @brief Initialize the SPI bus using the defined SPISettings.
	 */
	void beginTransaction(SPISettings& settings)
	{
		prepare(settings);
	}

	void beginTransaction(const SPISettings& settings)
	{
		SPISettings tmp{settings};
		prepare(tmp);
	}

	/**
	 * @brief Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	virtual void endTransaction()
	{
	}

	/**
	 * @brief Read one byte from SPI without setting up registers
	 * @param	none
	 * @retval	byte received
	 *
	 * Used for performance tuning when doing continuous reads
	 * this method does not reset the registers, so make sure
	 * that a regular transfer(data) call was performed
	 *
	 * Note: this method is not found on the Arduino API
	 *
	 * USE WITH CARE !!
	 *
	 */
	virtual uint8_t read8()
	{
		return transfer(0xff);
	}

	/**
	 * @name Send/receive some data
	 * @{
	 *
	 * SPI transfer is based on a simultaneous send and receive: the received data is returned in receivedVal (or receivedVal16).
	 * In case of buffer transfers the received data is stored in the buffer in-place (the old data is replaced with the data received).
	 *
	 * 		receivedVal = SPI.transfer(val)
	 * 		receivedVal16 = SPI.transfer16(val16)
	 * 		SPI.transfer(buffer, size)
	 */

	/**
	 * @brief Send/receive one byte of data
	 * @param val The byte to send
	 * @retval uint8_t The received byte
	 */
	uint8_t transfer(uint8_t val)
	{
		return transfer32(val, 8);
	}

	/**
	 * @brief Send/receive one 16-bit word of data
	 * @param val The word to send
	 * @retval uint16_t The received word
	 *
	 * Word is transferred either MSB first (bit 15) or LSB first (bit 0)
	 * depending on the currently applied bitOrder setting.
	 */
	uint16_t transfer16(uint16_t val)
	{
		return transfer32(val, 16);
	}

	/**
	 * @brief Send/receive a word of variable size
	 * @param val Word to send
	 * @param bits Size of word
	 *
	 * Word is transferred either MSB first (bits-1) or LSB first (bit 0)
	 * depending on the currently applied bitOrder setting.
	 */
	virtual uint32_t transfer32(uint32_t val, uint8_t bits = 32)
	{
		transfer(reinterpret_cast<uint8_t*>(&val), bits / 8);
		return val;
	}

	/**
	 * @brief Send/receive a variable-length block of data
	 * @param buffer IN: The data to send; OUT: The received data
	 * @param size Number of bytes to transfer
	 */
	virtual void transfer(uint8_t* buffer, size_t size) = 0;

	/** @} */

	/**
	 * @brief For testing, tie MISO <-> MOSI internally
	 *
	 * Note: Not included in std Arduino lib
	 */
	virtual bool loopback(bool enable) = 0;

	/**
	 * @brief  Default settings used by the SPI bus
	 * until reset by beginTransaction(SPISettings)
	 *
	 * Note: Not included in std Arduino lib
	 */
	SPISettings SPIDefaultSettings;

	const SpiPins& pins{mPins};

protected:
	/**
	 * @brief Prepare/configure with settings
	 * @param  settings include frequency, byte order and SPI mode
	 */
	virtual void prepare(SPISettings& settings) = 0;

	SpiPins mPins;
};

/** @} */
