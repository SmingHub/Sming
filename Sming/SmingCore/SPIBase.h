/*
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPIBase.h
 *
 *  Created on: Mar 2, 2016
 *      Author: harry-boe
 *
 */

#ifndef SMINGCORE_SPIBASE_H_
#define SMINGCORE_SPIBASE_H_

#include "SPISettings.h"


class SPIBase {
public:
	SPIBase();
	virtual ~SPIBase();

	/* Standard API
	 *
	 */

	/*
	 *  begin(): Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
	 */
	virtual void begin() = 0;

	/*
	 * end(): Disables the SPI bus (leaving pin modes unchanged).
	 */
	virtual void end() = 0;

	/*
	 * beginTransaction(): Initializes the SPI bus using the defined SPISettings.
	 */
	virtual void beginTransaction(SPISettings mySettings) = 0;

	/*
	 * endTransaction(): Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	virtual void endTransaction() = 0;

	/*
	 * transfer(), transfer16()
	 *
	 * SPI transfer is based on a simultaneous send and receive: the received data is returned in receivedVal (or receivedVal16). In case of buffer transfers the received data is stored in the buffer in-place (the old data is replaced with the data received).
	 *
	 * 		receivedVal = SPI.transfer(val)
	 * 		receivedVal16 = SPI.transfer16(val16)
	 * 		SPI.transfer(buffer, size)
	 */
	virtual unsigned char transfer(unsigned char val) = 0;
	virtual unsigned short transfer16(unsigned short val) = 0;
	virtual void transfer(uint8 * buffer, size_t size) = 0;

	SPISettings SPIDefaultSettings;

};

#endif /* SMINGCORE_SPIBASE_H_ */
