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

class SPISettings
{
public:
	SPISettings()
	{
#ifdef SPI_DEBUG
		debugf("SPISettings() default");
#endif
	}

	virtual ~SPISettings()
	{
	}

	/** @brief constructor for SPISettings
	 *
	 * Settings are applied to SPI::beginTransaction(SPISettings) and are valid until
	 * next beginTransaction()
	 *
	 * @param 	speed: The maximum speed of communication. For a SPI chip rated up to sys clock speed.
	 * For 20 MHz, use 20000000.
	 * @param	byteOrder: MSBFIRST or LSBFIRST
	 * @param	dataMode : SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3
	 *
	 * byteOrder's are:
	 *
	 * 		MSBFIRST 	Data is sent out starting with Bit31 and down to Bit0
	 * 		LSBFIRST 	Data is sent out starting with the lowest BYTE, from MSB to LSB.
	 *						0xABCDEFGH would be sent as 0xGHEFCDAB
	 *
	 * Data modes are:
	 *
	 *  		Mode		Clock Polarity (CPOL)	Clock Phase (CPHA)
	 * 		SPI_MODE0		0					0
	 * 		SPI_MODE1		0					1
	 * 		SPI_MODE2		1					0
	 * 		SPI_MODE3		1					1
	 */
	SPISettings(int speed, uint8_t byteOrder, uint8_t dataMode) : speed(speed), byteOrder(byteOrder), dataMode(dataMode)
	{
#ifdef SPI_DEBUG
		debugf("SPISettings(int %i, uint8 %d, uint8 %d)", speed, byteOrder, dataMode);
#endif
	}

	inline uint8_t getDataMode()
	{
		return dataMode;
	}

	// overload operator to check wheter the settings are equal
	bool operator==(const SPISettings& other) const
	{
		return (speed == other.speed) && (byteOrder == other.byteOrder) && (dataMode == other.dataMode);
	}

	void print(const char* s)
	{
#ifdef SPI_DEBUG
		debugf("->  %s -> SPISettings::print(int %i, uint8 %d, uint8 %d)", s, speed, byteOrder, dataMode);
#endif
	}

	friend class SPIClass;

private:
	int speed = 4000000;
	uint8_t byteOrder = MSBFIRST;
	uint8_t dataMode = SPI_MODE0;
};

/** @} */
