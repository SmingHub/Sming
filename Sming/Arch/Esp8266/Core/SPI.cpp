/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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
#include <esp_systemapi.h>
#include "espinc/eagle_soc.h"
#include "espinc/spi_register.h"

// define the static singleton
SPIClass SPI;

namespace
{
// Used internally to calculate optimum SPI speed
struct SpiPreDiv {
	unsigned freq;
	unsigned prescale;
	unsigned divisor;
};

/**
 * @brief Wait until HSPI has finished any current transaction
 */
__forceinline void spi_wait()
{
	while(READ_PERI_REG(SPI_CMD(SPI_NO)) & SPI_USR) {
		//
	}
}

/**
 * @brief Initiate an HSPI user transaction
 */
__forceinline void spi_send()
{
	SET_PERI_REG_MASK(SPI_CMD(SPI_NO), SPI_USR);
}

/**
 * @brief	Configure SPI mode parameters for clock edge and clock polarity.
 *
 *  		Private method used by SPISetings
 *
 * @param	SPI_MODE0 .. SPI_MODE4
 *
 * 	 		Mode		Clock Polarity (CPOL)	Clock Phase (CPHA)
 *			SPI_MODE0	0						0
 *			SPI_MODE1	0						1
 *			SPI_MODE2	1						0
 *			SPI_MODE3	1						1
 */
void spi_mode(uint8_t mode)
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

/**
 * @brief	Setup the byte order for shifting data out of buffer
 *
 *  		Private method used by SPISetings
 *
 * 	@param	MSBFIRST	1
 * 	 		Data is sent out starting with Bit31 and down to Bit0
 * 	 		LSBFIRST	0
 * 	 		Data is sent out starting with the lowest BYTE, from MSB to LSB
 * 			0xABCDEFGH would be sent as 0xGHEFCDAB
 */
void spi_byte_order(uint8_t byte_order)
{
#ifdef SPI_DEBUG
	debugf("SPIClass::spi_byte_order(byte_order %u)", byte_order);
#endif

	if(byte_order) {
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
		SET_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_RD_BYTE_ORDER);
	} else {
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_WR_BYTE_ORDER);
		CLEAR_PERI_REG_MASK(SPI_USER(SPI_NO), SPI_RD_BYTE_ORDER);
	}
}

/**
 * @brief Calculate the closest prescale value for a given frequency and clock-divider
 * @param  cpuFreq current CPU frequency, in Hz
 * @param  freq target SPI bus frequency, in Hz
 * @param  div divisor value to use
 * @retval SpiPreDiv contains resulting frequency, prescaler and divisor values
 */
SpiPreDiv calculateSpeed(unsigned cpuFreq, unsigned freq, unsigned div)
{
	SpiPreDiv prediv;
	unsigned pre = cpuFreq / (freq * div);
	if(pre == 0) {
		pre = 1;
	}
	unsigned n = pre * div;
	while(true) {
		prediv.freq = cpuFreq / n;
		if(prediv.freq <= freq) {
			break;
		}
		++pre;
		n += div;
	}
	prediv.prescale = pre;
	prediv.divisor = div;

#ifdef SPI_DEBUG
	debugf("SPI calculateSpeed(uint freq %u, uint pre %u, uint div %u)", f, pre, div);
#endif

	return prediv;
}

/** @brief Check speed settings and perform any pre-calculation required
 *  @param speed IN: requested bus frequency, OUT: Modified settings with prescale values
 *  @note
 *  		The algorithm is testing with clock dividers 2,3 and 5 to find the best pre-divider
 *  		The resulting clock frequency is not 100% accurate but delivers result within 5%
 *
 *  		It is guaranteed that the frequency will not exceed the given target
 *
 *  		Make sure that the ESP clock frequency is set before initializing the SPI bus.
 *  		Changes on the ESP clock are not recognised once initialized
 */
void checkSpeed(SPISpeed& speed)
{
	unsigned cpuFreq = system_get_cpu_freq() * 1000000UL;
#ifdef SPI_DEBUG
	debugf("SPIClass::calculateSpeed() -> current cpu frequency %u", cpuFreq);
#endif

	SpiPreDiv prediv;

	// If we're not running at max then need to determine appropriate prescale values
	if(speed.frequency >= cpuFreq) {
		// Use maximum speed
		prediv.freq = cpuFreq;
		prediv.divisor = 0;
		speed.regVal = SPI_CLK_EQU_SYSCLK;
	} else {
		prediv = calculateSpeed(cpuFreq, speed.frequency, 2);
		if(prediv.freq != speed.frequency) {
			// Use whichever divisor gives the highest frequency
			SpiPreDiv pd3 = calculateSpeed(cpuFreq, speed.frequency, 3);
			SpiPreDiv pd5 = calculateSpeed(cpuFreq, speed.frequency, 5);
			if(pd3.freq > prediv.freq || pd5.freq > prediv.freq) {
				prediv = (pd3.freq > pd5.freq) ? pd3 : pd5;
			}
		}

		// We have prescale and divisor values, now get regVal so we don't need to do this every time prepare() is called
		speed.regVal = (((prediv.prescale - 1) & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) |
					   (((prediv.divisor - 1) & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) |
					   (((prediv.divisor >> 1) & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) |
					   ((0 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S);
	}

	//#ifdef SPI_DEBUG
	debug_e("-> Using clock divider %u -> target freq %u -> result %u", prediv.divisor, speed.frequency, prediv.freq);
	//#endif

	speed.frequency = prediv.freq;
}

void spi_set_clock(SPISpeed& speed)
{
	// Clock register value is never 0, so indicates it hasn't been calculated
	if(speed.regVal == 0) {
		checkSpeed(speed);
	} else {
#ifdef SPI_DEBUG
		unsigned prescale = (speed.regVal >> SPI_CLKDIV_PRE_S) + 1;
		unsigned divisor = (speed.regVal >> SPI_CLKCNT_N_S) + 1;
		debugf("spi_set_clock(prescaler %u, divisor %u) for target %u", prescale, divisor, speed.frequency);
#endif
	}

	WRITE_PERI_REG(SPI_CLOCK(SPI_NO), speed.regVal);
}

} // namespace

bool SPIClass::begin()
{
	CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX, BIT9);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); // HSPIQ MISO == GPIO12
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // HSPID MOSI == GPIO13
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // CLK		 == GPIO14

	checkSpeed(SPIDefaultSettings.speed);
	prepare(SPIDefaultSettings);

	return true;
}

uint32_t SPIClass::transfer32(uint32_t data, uint8_t bits)
{
	uint32_t regvalue = READ_PERI_REG(SPI_USER(SPI_NO)) & (SPI_WR_BYTE_ORDER | SPI_RD_BYTE_ORDER | SPI_CK_OUT_EDGE);

	spi_wait();

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

	spi_send();
	spi_wait();

	auto res = READ_PERI_REG(SPI_W0(SPI_NO));
	if(READ_PERI_REG(SPI_USER(SPI_NO)) & SPI_RD_BYTE_ORDER) {
		res >>= (32 - bits);
	}

	return res;
}

uint8_t SPIClass::read8()
{
	spi_wait();

	WRITE_PERI_REG(SPI_W0(SPI_NO), 0x00);

	spi_send();
	spi_wait();

	auto res = READ_PERI_REG(SPI_W0(SPI_NO));
	if(READ_PERI_REG(SPI_USER(SPI_NO)) & SPI_RD_BYTE_ORDER) {
		res >>= 24;
	}

	return res;
}

void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
#define BLOCKSIZE 64U // the max length of the ESP SPI_W0 registers

	unsigned bufIndx = 0;

	unsigned blocks = ((numberBytes - 1) / BLOCKSIZE) + 1;
#ifdef SPI_DEBUG
	unsigned total = blocks;
#endif

	// loop number of blocks
	while(blocks--) {
		// get full BLOCKSIZE or number of remaining bytes
		auto bufLength = std::min(numberBytes - bufIndx, BLOCKSIZE);

#ifdef SPI_DEBUG
		debugf("Write/Read Block %u total %u bytes", total - blocks, bufLength);
#endif

		// compute the number of bits to clock
		auto num_bits = bufLength * 8;

		uint32_t regvalue = READ_PERI_REG(SPI_USER(SPI_NO)) & (SPI_WR_BYTE_ORDER | SPI_RD_BYTE_ORDER | SPI_CK_OUT_EDGE);

		spi_wait();

		regvalue |= SPI_USR_MOSI | SPI_DOUTDIN | SPI_CK_I_EDGE;
		WRITE_PERI_REG(SPI_USER(SPI_NO), regvalue);

		// setup bit length
		WRITE_PERI_REG(SPI_USER1(SPI_NO), (((num_bits - 1) & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S) |
											  (((num_bits - 1) & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S));

		// copy the registers starting from last index position
		if(IS_ALIGNED(buffer)) {
			memcpy((void*)SPI_W0(SPI_NO), &buffer[bufIndx], ALIGNUP4(bufLength));
		} else {
			uint32_t wordBuffer[BLOCKSIZE / 4];
			memcpy(wordBuffer, &buffer[bufIndx], bufLength);
			memcpy((void*)SPI_W0(SPI_NO), wordBuffer, ALIGNUP4(bufLength));
		}

		spi_send();
		spi_wait();

		// copy the registers starting from last index position
		memcpy(&buffer[bufIndx], (void*)SPI_W0(SPI_NO), bufLength);

		// increment bufIndex
		bufIndx += bufLength;
	}
}

void SPIClass::prepare(SPISettings& settings)
{
#ifdef SPI_DEBUG
	debugf("SPIClass::prepare(SPISettings)");
	settings.print("settings");
#endif

	//  setup clock
	spi_set_clock(settings.speed);

	//	set byte order
	spi_byte_order(settings.byteOrder);

	//	set spi mode
	spi_mode(settings.dataMode);
}
