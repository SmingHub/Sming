/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: harry-boe
 *
 *  Some code is derived from:
 *  	David Ogilvy (MetalPhreak)
 *
 ****/

#include "SPI.h"

#include <stdlib.h>
#include "eagle_soc.h"
#include "espinc/spi_register.h"
#include "c_types.h"

// define the static singleton
SPIClass SPI;

/* @defgroup SPI hardware implementation
 * @brief begin()
 *
 * Initializes the SPI bus using the default SPISettings
 *
 */
void SPIClass::begin()
{
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //clear bit9

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); // HSPIQ MISO == GPIO12
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // HSPID MOSI == GPIO13
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // CLK		 == GPIO14

	prepare(this->SPIDefaultSettings);
}

/* @defgroup SPI hardware implementation
 * @brief beginTransaction()
 *
 * Initializes the SPI bus using the defined SPISettings
 *
 * this methode does not initiate a transaction. So it can be used to
 * setup the SPI after SPI.begin()
 *
 */
void SPIClass::beginTransaction(SPISettings mySettings)
{
#ifdef SPI_DEBUG
	debugf("SPIhw::beginTransaction(SPISettings mySettings)");
#endif
	// check if we need to change settings
	if(this->spiSettings == mySettings) {
		return;
	}

	// prepare SPI settings
	prepare(mySettings);
}

/* @defgroup SPI hardware implementation
 * @brief transfer32()
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
uint32_t SPIClass::transfer32(uint32_t data, uint8_t bits)
{
	uint32_t regvalue = READ_PERI_REG(SPI_USER(SPI_NO)) & (SPI_WR_BYTE_ORDER | SPI_RD_BYTE_ORDER | SPI_CK_OUT_EDGE);

	while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR)
		;

	regvalue |= SPI_USR_MOSI | SPI_DOUTDIN | SPI_CK_I_EDGE;
	WRITE_PERI_REG(SPI_USER(SPI_NO), regvalue);

	WRITE_PERI_REG(SPI_USER1(SPI_NO), (((bits - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S) |
										  (((bits - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S));

	// copy data to W0
	if(READ_PERI_REG(SPI_USER(SPI_NO)) & SPI_WR_BYTE_ORDER) {
		WRITE_PERI_REG(SPI_W0(SPI_NO), data << (32 - bits));
	} else {
		WRITE_PERI_REG(SPI_W0(SPI_NO), data);
	}

	SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR); // send

	while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR)
		;

	// wait a while before reading the register into the buffer
	//	delayMicroseconds(2);

	if(READ_PERI_REG(SPI_USER(SPI_NO)) & SPI_RD_BYTE_ORDER) {
		return READ_PERI_REG(SPI_W0(SPI_NO)) >> (32 - bits); //Assuming data in is written to MSB. TBC
	} else {
		return READ_PERI_REG(SPI_W0(
			SPI_NO)); //Read in the same way as DOUT is sent. Note existing contents of SPI_W0 remain unless overwritten!
	}
}

/*
 * 	 used for performance tuning when doing continuous reads
 * 	 this method does not reset the registers , so make sure
 * 	 that a regular transfer(data) call was performed
 */
uint8_t SPIClass::read8()
{
	while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR)
		;

	WRITE_PERI_REG(SPI_W0(SPI_NO), 0x00);

	SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR); // send

	while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR)
		;

	if(READ_PERI_REG(SPI_USER(SPI_NO)) & SPI_RD_BYTE_ORDER) {
		return READ_PERI_REG(SPI_W0(SPI_NO)) >> (32 - 8); //Assuming data in is written to MSB. TBC
	} else {
		return READ_PERI_REG(SPI_W0(
			SPI_NO)); //Read in the same way as DOUT is sent. Note existing contents of SPI_W0 remain unless overwritten!
	}
}

/* @defgroup SPI hardware implementation
 * @brief transfer(uint8_t *buffer, size_t numberBytes)
 *
 * SPI transfer is based on a simultaneous send and receive:
 * The buffered transfers does split up the conversation internaly into 64 byte blocks.
 * The received data is stored in the buffer passed by reference.
 * (the data past in is replaced with the data received).
 *
 * 		SPI.transfer(buffer, size)				: memory buffer of length size
 */
void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
#define BLOCKSIZE 64 // the max length of the ESP SPI_W0 registers

	uint16 bufIndx = 0;

	int blocks = ((numberBytes - 1) / BLOCKSIZE) + 1;
#ifdef SPI_DEBUG
	int total = blocks;
#endif

	// loop number of blocks
	while(blocks--) {
		// get full BLOCKSIZE or number of remaining bytes
		uint8_t bufLength = std::min(numberBytes - bufIndx, (unsigned int)BLOCKSIZE);

#ifdef SPI_DEBUG
		debugf("Write/Read Block %d total %d bytes", total - blocks, bufLength);
#endif

		// compute the number of bits to clock
		uint8_t num_bits = bufLength * 8;

		uint32_t regvalue = READ_PERI_REG(SPI_USER(SPI_NO)) & (SPI_WR_BYTE_ORDER | SPI_RD_BYTE_ORDER | SPI_CK_OUT_EDGE);

		while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR)
			;

		regvalue |= SPI_USR_MOSI | SPI_DOUTDIN | SPI_CK_I_EDGE;
		WRITE_PERI_REG(SPI_USER(SPI_NO), regvalue);

		// setup bit lenght
		WRITE_PERI_REG(SPI_USER1(SPI_NO), (((num_bits - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S) |
											  (((num_bits - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S));

		// copy the registers starting from last index position
		memcpy((void*)SPI_W0(SPI_NO), &buffer[bufIndx], bufLength);

		// Begin SPI Transaction
		SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR);

		// wait for SPI bus to be ready
		while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR)
			;

		// wait a while before reading the register into the buffer
		//		delayMicroseconds(8);

		// copy the registers starting from last index position
		memcpy(&buffer[bufIndx], (void*)SPI_W0(SPI_NO), bufLength);

		// increment bufIndex
		bufIndx += bufLength;
	}
};

/** @defgroup SPI hardware implementation
 * @brief  prepare	apply SPI bus settings
 *
 * 		Private method used by beginTransaction and begin (init)
 *
 * 	@param  SPISettings include frequency, byte order and SPI mode
 */
void SPIClass::prepare(SPISettings mySettings)
{
#ifdef SPI_DEBUG
	debugf("SPIClass::prepare(SPISettings mySettings)");
	mySettings.print("mySettings");
#endif

	// check if we need to change settings
	if(initialised && spiSettings == mySettings)
		return;

	//  setup clock
	setFrequency(mySettings.speed);

	//	set byte order
	this->spi_byte_order(mySettings.byteOrder);

	//	set spi mode
	spi_mode(mySettings.dataMode);

#ifdef SPI_DEBUG
	debugf("SPIhw::prepare(SPISettings mySettings) -> updated settings");
#endif

	spiSettings = mySettings;
	initialised = true;
};

/** @brief  spi_mode Configures SPI mode parameters for clock edge and clock polarity.
 *
 *  		Private method used by SPISetings
 *
 * 	@param	SPI_MODE0 .. SPI_MODE4
 *
 * 	 		Mode		Clock Polarity (CPOL)	Clock Phase (CPHA)
 *			SPI_MODE0	0						0
 *			SPI_MODE1	0						1
 *			SPI_MODE2	1						0
 *			SPI_MODE3	1						1
 */
void SPIClass::spi_mode(uint8_t mode)
{
	uint8_t spi_cpha = mode & 0x0F;
	uint8_t spi_cpol = mode & 0xF0;

#ifdef SPI_DEBUG
	debugf("SPIClass::spi_mode(mode %x) spi_cpha %X,spi_cpol %X)", mode, spi_cpha, spi_cpol);
#endif

	if(spi_cpha == spi_cpol) {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_CK_OUT_EDGE);
	} else {
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_CK_OUT_EDGE);
	}

	if(spi_cpol) {
		SET_PERI_REG_MASK(SPI_PIN(SPI_NO), SPI_IDLE_EDGE);
	} else {
		CLEAR_PERI_REG_MASK(SPI_PIN(SPI_NO), SPI_IDLE_EDGE);
	}
}

/** @brief  spi_byte_order Setup the byte order for shifting data out of buffer
 *
 *  		Private method used by SPISetings
 *
 * 	@param	MSBFIRST	1
 * 	 		Data is sent out starting with Bit31 and down to Bit0
 * 	 		LSBFIRST	0
 * 	 		Data is sent out starting with the lowest BYTE, from MSB to LSB
 * 			0xABCDEFGH would be sent as 0xGHEFCDAB
 */
void SPIClass::spi_byte_order(uint8_t byte_order)
{
#ifdef SPI_DEBUG
	debugf("SPIClass::spi_byte_order(byte_order %d)", byte_order);
#endif

	if(byte_order) {
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_RD_BYTE_ORDER);
	} else {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_RD_BYTE_ORDER);
	}
}

/** @brief  setClock  set CPI bus clock
 *
 * 		Private method used by setFrequency
 *
 * 		time length HIGHT level = (CPU clock / 10 / 2) ^ -1,
 * 		time length LOW level = (CPU clock / 10 / 2) ^ -1
 * 		Frequency calculation: 80Mhz / predivider / divider
 *
 * 	@param  prediv  time length HIGHT level
 * 	@param  prediv  time length LOW level
 *
 */
void SPIClass::setClock(uint8_t prediv, uint8_t cntdiv)
{
#ifdef SPI_DEBUG
	debugf("SPIClass::setClock(prediv %d, cntdiv %d) for target %d", prediv, cntdiv, spiSettings.speed);
#endif
	debugf("SPIClass::setClock(prediv %d, cntdiv %d) for target %d", prediv, cntdiv, spiSettings.speed);

	if((prediv == 0) | (cntdiv == 0)) {
		// go full speed = SYSTEMCLOCK
		WRITE_PERI_REG(SPI_CLOCK(SPI_NO), SPI_CLK_EQU_SYSCLK);
	} else {
		WRITE_PERI_REG(SPI_CLOCK(SPI_NO), (((prediv - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
											  (((cntdiv - 1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
											  (((cntdiv >> 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
											  ((0 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
	}
}

/** @brief  gnu lib div implementation
 *          TODO: check whether there is a implementation in SMING already
 */
div_t div(int numer, int denom)
{
	div_t result;
	result.quot = numer / denom;
	result.rem = numer % denom;
	if(numer >= 0 && result.rem < 0) {
		++result.quot;
		result.rem -= denom;
	}
	return result;
}

/** @brief  getFrequency computes the closest pre devider for a given clock-devider and frequency
 *
 * 	Private method used by setFrequency
 *
 *  @param  freq target SPI bus frequency
 *  @param  &pre return the computed pre devider (pass by reference)reqHelp Help message shown by CLI "help" command
 *  @param  clk  clock devider
 */
uint32_t SPIClass::getFrequency(int freq, int& pre, int clk)
{
	int _CPU_freq = system_get_cpu_freq() * 1000000UL;
#ifdef SPI_DEBUG
	debugf("SPIClass::getFrequency -> current cpu frequency %d", _CPU_freq);
#endif

	int divider = _CPU_freq / freq;

	div_t divresult = div(divider, clk);
	pre = divresult.quot;

	int f = _CPU_freq / pre / clk;
	while(f > freq) {
		pre++;
		f = _CPU_freq / pre / clk;
	}
#ifdef SPI_DEBUG
	debugf("SPIClass::getFrequency(int freq %d, int &pre %d, int clk %d)", f, pre, clk);
#endif
	return f;
}

/** @brief  set the max SPI bus frequency
 *
 *  		Private method used by SPISetings
 *
 *  		The algorithm is testing with colock deviders 2,3 and 5 to find the best pre-devider
 *  		The resulting clock frequency is not 100% accurate but delivers result within 5%
 *
 *  		It is guaranteed that the frequency will not exceed the given target
 *
 *  		Make sure that the ESP clock frequency is set before initializing the SPI bus.
 *  		Changes on the ESP clock are not recognised once initialized
 *
 *  @param  freq Max SPI bus frequency
 * @{
 */
void SPIClass::setFrequency(int freq)
{
#ifdef SPI_DEBUG
	debugf("SPIClass::setFrequency(uint32_t %d)", freq);
#endif

	int _CPU_freq = system_get_cpu_freq() * 10000000UL;

	// dont run code if there are no changes
	if(initialised && freq == spiSettings.speed)
		return;

	if(freq == _CPU_freq) {
		spiSettings.speed = freq;
		setClock(0, 0);
		return;
	}

	freq = std::min(freq, _CPU_freq / 2);
	spiSettings.speed = freq;

	int pre2;
	int f2 = getFrequency(freq, pre2, 2);
	if(f2 == freq) {
#ifdef SPI_DEBUG
		debugf("-> Hit!! -> target freq %d -> result %d", freq, _CPU_freq / pre2 / 2);
#endif
		setClock(pre2, 2);
		return;
	}

	int pre3;
	int f3 = getFrequency(freq, pre3, 3);
	int pre5;
	int f5 = getFrequency(freq, pre5, 5);
	if(f3 <= f2 && f2 >= f5) {
#ifdef SPI_DEBUG
		debugf("-> Using clock divider 2 -> target freq %d -> result %d", freq, _CPU_freq / pre2 / 2);
#endif
		setClock(pre2, 2);
		return;
	} else {
		if(f5 <= f3) {
#ifdef SPI_DEBUG
			debugf("-> Using clock divider 3 -> target freq %d -> result %d", freq, _CPU_freq / pre3 / 3);
#endif
			setClock(pre3, 3);
			return;
		} else {
#ifdef SPI_DEBUG
			debugf("-> Using clock divider 5 -> target freq %d -> result %d", freq, _CPU_freq / pre5 / 5);
#endif
			setClock(pre5, 5);
			return;
		}
	}
}
