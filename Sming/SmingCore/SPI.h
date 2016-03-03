/*
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.h
 *
 *  Created on: Mar 2, 2016
 *      Author: harry-boe
 */

#ifndef SMINGCORE_SPI_H_
#define SMINGCORE_SPI_H_

#include "SPIBase.h"
#include "SPISettings.h"


#define SPI_DEBUG  1

// for compatibility when porting from Arduino
#define SPI_HAS_TRANSACTION  1

#define SPI_CONTINUE	1

#define	SPI_NO	1

//Define some default SPI clock settings
#define SPI_CLK_PREDIV 10
#define SPI_CLK_CNTDIV 2

// Byte Order definitions
#define SPI_BYTE_ORDER_HIGH_TO_LOW 1
#define SPI_BYTE_ORDER_LOW_TO_HIGH 0


class SPIhw: public SPIBase {
public:
	SPIhw();
	virtual ~SPIhw();

	/*
	 * Arduino Standard API
	 */
	/* Standard API
	 *
	 */

	/*
	 *  begin(): Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
	 */
	void begin();

	/*
	 * end(): Disables the SPI bus (leaving pin modes unchanged).
	 */
	void end();

	/*
	 * beginTransaction(): Initializes the SPI bus using the defined SPISettings.
	 */
	void beginTransaction(SPISettings mySettings);

	/*
	 * endTransaction(): Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	void endTransaction();

	/*
	 * transfer(), transfer16()
	 *
	 * SPI transfer is based on a simultaneous send and receive: the received data is returned in receivedVal (or receivedVal16). In case of buffer transfers the received data is stored in the buffer in-place (the old data is replaced with the data received).
	 *
	 * 		receivedVal = SPI.transfer(val)
	 * 		receivedVal16 = SPI.transfer16(val16)
	 * 		SPI.transfer(buffer, size)
	 */
	virtual unsigned char transfer(unsigned char val);
//	{
//		return (unsigned char) transfer32((uint32) val, 1);
//	};
	virtual unsigned short transfer16(unsigned short val) {
		return (unsigned short) transfer32((uint32) val, 2);
	};
	virtual void transfer(uint8 * buffer, size_t numberBytes);


	/*
	 * Extended Arduino DUO API: enabling CS pin usage
	 */
	void begin(uint8 CS_PIN);

	uint8 transfer(uint8 CS_PIN, uint8 val);
	uint16 transfer16(uint8 CS_PIN, uint16 val);
	void transfer(uint8 CS_PIN, uint8 * buffer, size_t size);

	// same with transaction HANDLING
	uint8 transfer(uint8 CS_PIN, uint8 val, bool endTX);
	uint16 transfer16(uint8 CS_PIN, uint16 val, bool endTX);
	void transfer(uint8 CS_PIN, uint8 * buffer, size_t size, bool endTX);

private:

	// prepare/configure HSPI with settings
	void prepare(SPISettings mySettings);
	uint32 transfer32(uint32 val, uint8 bytes);

	uint8 _CS_PIN;
	SPISettings _SPISettings;
	uint8	_isTX = false;
	uint8	_init = false;

};

extern SPIhw SPI;

#endif /* SMINGCORE_SPI_H_ */
