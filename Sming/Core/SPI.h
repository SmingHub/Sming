/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.h
 *
 *  Created on: Mar 2, 2016
 *      Author: harry-boe
 *
 */

/** @defgroup hw_spi SPI Hardware support
 *  @brief    Provides hardware SPI support
 */

#pragma once

#include "SPIBase.h"
#include "SPISettings.h"

//#define SPI_DEBUG  1

// for compatibility when porting from Arduino
#define SPI_HAS_TRANSACTION 0

#define SPI_NO 1

/**
 * @brief  Hardware SPI object
 * @addtogroup hw_spi
 * @{
 */

class SPIClass : public SPIBase
{
public:
	void begin() override;

	/**
	 * @brief Method for compatibility with Arduino API. Provides NOP
	 */
	void end() override
	{
	}

	/**
	 * @brief Check provided speed settings and perform pre-calculation
	 * @param settings IN: requested bus settings, OUT: Modified bus settings
	 * @note
	 *  		This method allows clients to pre-calculate bus speed settings, so
	 *  		may return with a lower bus frequency than requested.
	 *
	 *  		The algorithm is testing with clock dividers 2,3 and 5 to find the best pre-divider
	 *  		The resulting clock frequency is not 100% accurate but delivers result within 5%
	 *
	 *  		It is guaranteed that the frequency will not exceed the given target
	 *
	 *  		Make sure that the ESP clock frequency is set before initializing the SPI bus.
	 *  		Changes on the ESP clock are not recognised once initialized
	 */
	static void checkSpeed(SPISpeed& speed);

	/**
	 * @brief Initialize the SPI bus using the defined SPISettings
	 *
	 * This method does not initiate a transaction, so it can be used to
	 * setup the SPI after SPI.begin()
	 *
	 */
	void beginTransaction(SPISettings& mySettings) override;

	/**
	 * @brief Method for compatibility with Arduino API. Provides NOP
	 */
	void endTransaction() override
	{
#ifdef SPI_DEBUG
		debugf("SPIhw::endTransaction()");
#endif
	}

	uint8_t transfer(uint8_t val) override
	{
		return transfer32(val, 8);
	}

	/** @brief Read one byte from SPI without setting up registers
	 * 	@param	none
	 * 	@retval	byte received
	 *
	 * 	 used for performance tuning when doing continuous reads
	 * 	 this method does not reset the registers , so make sure
	 * 	 that a regular transfer(data) call was performed
	 *
	 * 	 Note: this method is not found on the Arduino API
	 *
	 * 	 USE WITH CARE !!
	 *
	 */
	uint8_t read8();

	uint16_t transfer16(uint16_t val) override
	{
		return transfer32(val, 16);
	}

	void transfer(uint8_t* buffer, size_t numberBytes) override;

	/**
	 * @brief  Default settings used by the SPI bus
	 * until reset by beginTransaction(SPISettings)
	 *
	 * Note: not included in std Arduino lib
	 */
	SPISettings SPIDefaultSettings;

private:
	/**
	 * @brief Send/receive a word of variable size
	 * @param val Word to send
	 * @param bits Number of bits to send
	 *
	 * private method used by transfer(byte) and transfer16(sort)
	 * to send/recv one uint32_t
	 *
	 * SPI transfer is based on a simultaneous send and receive:
	 * the received data is returned in receivedVal (or receivedVal16).
	 *
	 * 		receivedVal = SPI.transfer(val)			: single byte
	 * 		receivedVal16 = SPI.transfer16(val16)	: single short
	 */
	virtual uint32_t transfer32(uint32_t val, uint8_t bits);

	/**
	 * @brief Prepare/configure HSPI with settings
	 * @param  settings include frequency, byte order and SPI mode
	 *
	 * Private method used by beginTransaction and begin (init)
	 *
	 */
	void prepare(SPISettings& settings);
};

/** @brief  Global instance of SPI class */
extern SPIClass SPI;
