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
/** @defgroup hw_spi SPI Hardware support
 *  @brief    Provides hardware SPI support
 */

#ifndef SMINGCORE_SPI_H_
#define SMINGCORE_SPI_H_

#include "SPIBase.h"
#include "SPISettings.h"


//#define SPI_DEBUG  1

// for compatibility when porting from Arduino
#define SPI_HAS_TRANSACTION  0

#define	SPI_NO	1


class SPIClass: public SPIBase {
public:
    /** @brief  Instantiate hardware SPI object
     *  @addtogroup hw_spi
     *  @{
     */
	SPIClass();
	virtual ~SPIClass();

	/* @brief begin()
	 *
	 * Initializes the SPI bus using the default SPISettings
	 */
	virtual void begin();


	/** @brief end()
	 *
	 * Method for compatibility with Arduino API. Provides NOP
	 *
	 */
	virtual void end();

	/** @brief beginTransaction()
	 *
	 * Initializes the SPI bus using the defined SPISettings
	 *
	 * this methode does not initiate a transaction. So it can be used to
	 * setup the SPI after SPI.begin()
	 *
	 */
	virtual void beginTransaction(SPISettings mySettings);

	/** @brief endTransaction()
	 *
	 * Method for compatibility with Arduino API. Provides NOP
	 *
	 * endTransaction(): Stop using the SPI bus. Normally this is called after
	 * de-asserting the chip select, to allow other libraries to use the SPI bus.
	 */
	virtual void endTransaction();

	/** @brief 	transfer()
	 * @param	byte to send
	 * @retval	byte received
	 *
	 * calls private method transfer32(byte) to send/recv one uint32_t
	 * input/output casted to rightdta type
	 *
	 * SPI transfer is based on a simultaneous send and receive:
	 * the received data is returned in receivedVal (or receivedVal16).
	 *
	 * 		receivedVal = SPI.transfer(val)			: single byte
	 * 		receivedVal16 = SPI.transfer16(val16)	: single short
	 */
	virtual unsigned char transfer(unsigned char val) {
		return transfer32((uint32)val, 8);
	};

	/** @brief read8() read a byte from SPI without setting up registers
	 * @param	none
	 * @retval	byte received
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
	uint8 read8();

	/** @brief 	transfer16()
	 * @param	short to send
	 * @retval	short received
	 *
	 * calls private method transfer32(byte) to send/recv one uint32_t
	 * input/output casted to rightdta type
	 *
	 * SPI transfer is based on a simultaneous send and receive:
	 * the received data is returned in receivedVal (or receivedVal16).
	 *
	 * 		receivedVal = SPI.transfer(val)			: single byte
	 * 		receivedVal16 = SPI.transfer16(val16)	: single short
	 */
	virtual unsigned short transfer16(unsigned short val) {
		return transfer32((uint32)val, 16);
	};

	/** @brief 	transfer(uint8 *buffer, size_t numberBytes)
	 * @param	buffer in/out
	 * @param	numberBytes lenght of buffer
	 *
	 * SPI transfer is based on a simultaneous send and receive:
	 * The buffered transfers does split up the conversation internaly into 64 byte blocks.
	 * The received data is stored in the buffer passed by reference.
	 * (the data past in is replaced with the data received).
	 *
	 * 		SPI.transfer(buffer, size)				: memory buffer of length size
	 */
	virtual void transfer(uint8 * buffer, size_t numberBytes);


	/** @brief  Default settings used by the SPI bus
	 * until reset by beginTransaction(SPISettings)
	 *
	 * Note: not included in std Arduino lib
	 */
	SPISettings SPIDefaultSettings = SPISettings(4000000, MSBFIRST, SPI_MODE0);


private:

	/** @brief transfer32()
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
	virtual uint32 transfer32(uint32 val, uint8 bits);

	// prepare/configure HSPI with settings
	void prepare(SPISettings mySettings);

	/** @brief  spi_byte_order
	 * private method used when applying SPISettings
	 */
	void spi_byte_order(uint8 byte_order);

	/** @brief  spi_mode
	 * private method used when applying SPISettings
	 */
	void spi_mode(uint8 mode);

	/** @brief  setClock
	 * private method used when applying SPISettings
	 */
	void setClock(uint8 prediv, uint8 cntdiv);

	/** @brief  setClock
	 * private method used when applying SPISettings
	 */
	uint32_t getFrequency(int freq, int &pre, int clk);
	void setFrequency(int freq);

	SPISettings _SPISettings;
	uint8	_isTX = false;
	uint8	_init = false;

};

/** @brief  Global instance of SPI class */
extern SPIClass SPI;

#endif /* SMINGCORE_SPI_H_ */
