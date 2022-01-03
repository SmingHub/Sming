/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPISettings.h
 *
 *  Created on: Mar 2, 2016
 *      Author: harry
 *
 ****/

#pragma once

#include "Digital.h"
#ifdef SPI_DEBUG
#include <debug_progmem.h>
#endif

/** @ingroup base_spi
 *  @{
 */

// Mode			Clock Polarity (CPOL)	Clock Phase (CPHA)
//	SPI_MODE0		0					0
//	SPI_MODE1		0					1
//	SPI_MODE2		1					0
//	SPI_MODE3		1					1

#define SPI_MODE0 0x00
#define SPI_MODE1 0x0F
#define SPI_MODE2 0xF0
#define SPI_MODE3 0xFF

const uint32_t SPI_SPEED_DEFAULT = 4000000UL;

/** @brief Contains bus frequency and corresponding divisor/prescaler settings
 *  @note Pre-calculating divisor settings improves performance when switching
 *  between bus devices
 */
struct SPISpeed {
	uint32_t frequency{0};
	uint32_t regVal{0}; ///< Cached clock register value

	SPISpeed(uint32_t freq = SPI_SPEED_DEFAULT) : frequency(freq)
	{
	}

	SPISpeed& operator=(uint32_t freq)
	{
		setFrequency(freq);
		return *this;
	}

	bool operator==(const SPISpeed& speed) const
	{
		return frequency == speed.frequency;
	}

	void setFrequency(uint32_t freq)
	{
		if(freq != frequency) {
			frequency = freq;
			regVal = 0;
		}
	}
};

class SPISettings
{
public:
	SPISettings()
	{
#ifdef SPI_DEBUG
		debugf("SPISettings::SPISettings() default");
#endif
	}

	/** @brief constructor for SPISettings
	 *
	 * Settings are applied to SPI::beginTransaction(SPISettings) and are valid until
	 * next beginTransaction()
	 *
	 * @param 	speed: The maximum speed of communication. For a SPI chip rated up to sys clock speed.
	 * For 20 MHz, use 20000000.
	 * @param	bitOrder: MSBFIRST or LSBFIRST
	 * 			Determines how bits within each byte are sent on the wire.
	 * 			Data is always sent LSB first (matches system endianness)
	 * @param	dataMode : SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3
	 *
	 *  		Mode		Clock Polarity (CPOL)	Clock Phase (CPHA)
	 * 		SPI_MODE0		0					0
	 * 		SPI_MODE1		0					1
	 * 		SPI_MODE2		1					0
	 * 		SPI_MODE3		1					1
	 */
	SPISettings(uint32_t speed, uint8_t bitOrder, uint8_t dataMode)
		: speed(speed), bitOrder(bitOrder), dataMode(dataMode)
	{
#ifdef SPI_DEBUG
		debugf("SPISettings(int %i, uint8 %u, uint8 %u)", speed, bitOrder, dataMode);
#endif
	}

	// overload operator to check whether the settings are equal
	bool operator==(const SPISettings& other) const
	{
		return (speed == other.speed) && (bitOrder == other.bitOrder) && (dataMode == other.dataMode);
	}

	void print(const char* s)
	{
#ifdef SPI_DEBUG
		debugf("->  %s -> SPISettings(%u, %u, %u)", s, speed.frequency, bitOrder, dataMode);
#endif
	}

	SPISpeed speed;
	uint8_t bitOrder{MSBFIRST};
	uint8_t dataMode{SPI_MODE0};
};

/** @} */
