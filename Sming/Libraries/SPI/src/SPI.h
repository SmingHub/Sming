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

#pragma once

#include "SPIBase.h"
#include "SPISettings.h"
#include <spi_arch.h>

/**
 * @defgroup hw_spi SPI Hardware support
 * @brief    Provides hardware SPI support
 * @{
 */

/**
 * @brief  Hardware SPI class
 */
class SPIClass : public SPIBase
{
public:
	SPIClass();
	SPIClass(const SPIClass&) = delete;
	SPIClass& operator=(const SPIClass&) = delete;

	/**
	 * @brief Alternative to defining bus and pin set in constructor.
	 * Use this method to change global `SPI` instance setup.
	 *
	 * IMPORTANT: Must be called *before* begin().
	 */
	bool setup(SpiBus id, SpiPins pins = {});

	bool setup(SpiPins pins)
	{
		return setup(SpiBus::DEFAULT, pins);
	}

	bool begin() override;
	void end() override;

	uint8_t read8() override;
	uint32_t transfer32(uint32_t val, uint8_t bits = 32) override;

	using SPIBase::transfer;
	void transfer(uint8_t* buffer, size_t numberBytes) override;

	bool loopback(bool enable) override;

protected:
	void prepare(SPISettings& settings) override;

private:
#ifdef ARCH_ESP32
	SpiBus busId{SpiBus::DEFAULT};
#endif
	bool lsbFirst{false};
};

/** @brief  Global instance of SPI class */
extern SPIClass SPI;

/** @} */
