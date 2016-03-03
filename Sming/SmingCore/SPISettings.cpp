/*
 * SPISettings.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: harry
 */

#include "SPISettings.h"
#include "SPI.h"

#define	SPI_MODE0		0x00
#define	SPI_MODE1		0x0F
#define	SPI_MODE2		0xF0
#define	SPI_MODE3		0xFF


SPISettings::SPISettings() {
//#ifdef SPI_DEBUG
//	debugf("SPISettings::SPISettings()");
//#endif
	_speed = 400000;
	_byteOrder = SPI_BYTE_ORDER_LOW_TO_HIGH;
	_dataMode = SPI_MODE0;
}

/*
 * 	speedMaximum: The maximum speed of communication. For a SPI chip rated up to 20 MHz, use 20000000.
 * 	dataOrder: MSBFIRST or LSBFIRST
 * 	dataMode : SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3
 */
SPISettings::SPISettings(int speed, uint8 byteOrder, uint8 dataMode) {

//#ifdef SPI_DEBUG
//	debugf("SPISettings::SPISettings(int %i, uint8 %d, uint8 %d)", speed, byteOrder, dataMode);
//#endif

	_speed = speed;
	_byteOrder = byteOrder;
	_dataMode = dataMode;
}


SPISettings::~SPISettings() {
	// TODO Auto-generated destructor stub
}


bool SPISettings::operator==(const SPISettings &other) const {


	int res = _speed == other._speed &
			_byteOrder == other._byteOrder &
			_dataMode == other._dataMode;

//#ifdef SPI_DEBUG
//	debugf("SPISettings::operator==(const SPISettings &other) = %x", res);
//	debugf("--> speed %i %i order %d %d mode %d %d", _speed, other._speed, _byteOrder, other._byteOrder, _dataMode, other._dataMode );
//#endif

	return res;

}

//void SPISettings::print(String s) {
//		debugf("->  %s -> SPISettings::print(int %i, uint8 %d, uint8 %d)", s.c_str(), _speed, _byteOrder, _dataMode);
//}



