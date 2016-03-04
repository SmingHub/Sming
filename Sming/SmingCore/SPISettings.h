/*
 * SPISettings.h
 *
 *  Created on: Mar 2, 2016
 *      Author: harry
 */

#ifndef SMINGCORE_SPISETTINGS_H_
#define SMINGCORE_SPISETTINGS_H_

#include "Digital.h"

// Mode			Clock Polarity (CPOL)	Clock Phase (CPHA)
//	SPI_MODE0		0					0
//	SPI_MODE1		0					1
//	SPI_MODE2		1					0
//	SPI_MODE3		1					1

#define	SPI_MODE0		0x00
#define	SPI_MODE1		0x0F
#define	SPI_MODE2		0xF0
#define	SPI_MODE3		0xFF

class SPISettings {
public:
	SPISettings();
	virtual ~SPISettings();

	/*
	 * 	speedMaximum: The maximum speed of communication. For a SPI chip rated up to 20 MHz, use 20000000.
	 * 	dataOrder: MSBFIRST or LSBFIRST
	 * 	dataMode : SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3
	 */
	SPISettings(int speed, uint8 byteOrder, uint8 dataMode);
	inline uint8 getDataMode() {return _dataMode;};

	// overload operator to check wheter the settings are equal
	bool operator==(const SPISettings &other) const;

//	void print(String s);

	friend class SPIClass;

private:
	int 	_speed;
	uint8	_byteOrder;
	uint8	_dataMode;
};

#endif /* SMINGCORE_SPISETTINGS_H_ */
