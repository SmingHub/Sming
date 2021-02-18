/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.h
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

/**
 * @brief  Hardware SPI object
 * @addtogroup hw_spi
 * @{
 */

class SPIClass : public SPIBase
{
public:
	bool begin() override;

	void end() override
	{
	}

	using SPIBase::beginTransaction;
	using SPIBase::transfer;
	void transfer(uint8_t* buffer, size_t numberBytes) override;

protected:
	void prepare(SPISettings& settings) override;
};

/** @brief  Global instance of SPI class */
extern SPIClass SPI;
