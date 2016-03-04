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
#define SPI_HAS_TRANSACTION  0

#define SPI_CONTINUE	1

#define	SPI_NO	1

//Define some default SPI clock settings
#define SPI_CLK_PREDIV 10
#define SPI_CLK_CNTDIV 2

// Byte Order definitions
#define SPI_BYTE_ORDER_HIGH_TO_LOW 1
#define SPI_BYTE_ORDER_LOW_TO_HIGH 0


class SPIClass: public SPIBase {
public:
	SPIClass();
	virtual ~SPIClass();

	/*
	 * Arduino Standard API
	 */
	/* Standard API
	 *
	 */

	/*
	 *  begin(): Initializes the SPI bus by setting SCK, MOSI, and SS to outputs, pulling SCK and MOSI low, and SS high.
	 */
	virtual void begin();

	void begin(uint16_t predivider, uint8_t divider);

	/*
	 * end(): Disables the SPI bus (leaving pin modes unchanged).
	 */
	virtual void end();

	/*
	 * beginTransaction(): Initializes the SPI bus using the defined SPISettings.
	 */
	virtual void beginTransaction(SPISettings mySettings);

	/*
	 * endTransaction(): Stop using the SPI bus. Normally this is called after de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	virtual void endTransaction();

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

	SPISettings SPIDefaultSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0);


private:

	// methods
	void setClock(uint8 prediv, uint8 cntdiv);
	uint32_t getFrequency(int freq, uint8 &pre, uint8 clk);
	void setFrequency(uint32_t freq);

	// prepare/configure HSPI with settings
	void prepare(SPISettings mySettings);


	uint32 transfer32(uint32 val, uint8 bytes);

	SPISettings _SPISettings;
	uint8	_isTX = false;
	uint8	_init = false;

};

extern SPIClass SPI;

#endif /* SMINGCORE_SPI_H_ */
