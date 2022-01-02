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
#include "espinc/spi_struct.h"

#define GET_DEVICE(err)                                                                                                \
	if(!busAssigned) {                                                                                                 \
		debug_e("[SPI] Not Ready");                                                                                    \
		return err;                                                                                                    \
	}                                                                                                                  \
	SpiDevice dev;

SPIClass SPI;

namespace
{
constexpr size_t SPI_FIFO_SIZE{64};

bool busAssigned;

// Used internally to calculate optimum SPI speed
struct SpiPreDiv {
	unsigned freq;
	unsigned prescale;
	unsigned divisor;
};

struct SpiDevice {
	volatile spi_dev_t* hw;

	SpiDevice() : hw(&SPI1)
	{
	}

	void init()
	{
		hw->user.val = 0;
		hw->user.usr_mosi = true;
		hw->user.duplex = true;
		hw->user.ck_i_edge = true;
	}

	void set_clock(SPISpeed& speed);

	/**
	 * @brief Wait until SPI has finished any current transaction
	 */
	void wait()
	{
		while(hw->cmd.usr) {
			//
		}
	}

	/**
	 * @brief Initiate an SPI user transaction
	 */
	void send(unsigned num_bits)
	{
		hw->user1.usr_mosi_bitlen = num_bits - 1;
		hw->cmd.usr = true;
	}

	void set_mode(uint8_t mode)
	{
		uint8_t spi_cpha = mode & 0x0F;
		uint8_t spi_cpol = mode & 0xF0;

#ifdef SPI_DEBUG
	debugf("[SPI] spi_mode(mode %x) spi_cpha %X,spi_cpol %X)", mode, spi_cpha, spi_cpol);
#endif

		hw->user.ck_out_edge = (spi_cpha != spi_cpol);
		hw->pin.ck_idle_edge = spi_cpol;
	}

	void set_bit_order(uint8_t bit_order)
	{
#ifdef SPI_DEBUG
		debugf("[SPI] set_bit_order(bit_order %u)", bit_order);
#endif
		hw->ctrl.rd_bit_order = (bit_order != MSBFIRST);
		hw->ctrl.wr_bit_order = (bit_order != MSBFIRST);
	}

	uint32_t read()
	{
		return hw->data_buf[0];
	}

	void write(uint32_t value)
	{
		hw->data_buf[0] = value;
	}

	void read(void* buffer, size_t length)
	{
		memcpy(buffer, (void*)hw->data_buf, length);
	}

	void write(const void* buffer, size_t length)
	{
		if(IS_ALIGNED(buffer)) {
			memcpy((void*)hw->data_buf, buffer, ALIGNUP4(length));
		} else {
			uint32_t wordBuffer[SPI_FIFO_SIZE];
			memcpy(wordBuffer, buffer, length);
			memcpy((void*)hw->data_buf, wordBuffer, ALIGNUP4(length));
		}
	}
};

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
	debugf("[SPI] calculateSpeed(uint freq %u, uint pre %u, uint div %u)", f, pre, div);
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
	debugf("[SPI] calculateSpeed() -> current cpu frequency %u", cpuFreq);
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
		decltype(spi_dev_t::clock) clk{{
			.clkcnt_h = prediv.divisor - 1,
			.clkcnt_n = prediv.divisor - 1,
			.clkdiv_pre = prediv.prescale - 1,
		}};
		speed.regVal = clk.val;
	}

#ifdef SPI_DEBUG
	debugf("[SPI] Using clock divider %u -> target freq %u -> result %u", prediv.divisor, speed.frequency, prediv.freq);
#endif

	speed.frequency = prediv.freq;
}

void SpiDevice::set_clock(SPISpeed& speed)
{
	// Clock register value is never 0, so indicates it hasn't been calculated
	if(speed.regVal == 0) {
		checkSpeed(speed);
	} else {
#ifdef SPI_DEBUG
		unsigned prescale = (speed.regVal >> SPI_CLKDIV_PRE_S) + 1;
		unsigned divisor = (speed.regVal >> SPI_CLKCNT_N_S) + 1;
		debugf("[SPI] set_clock(prescaler %u, divisor %u) for target %u", prescale, divisor, speed.frequency);
#endif
	}

	hw->clock.val = speed.regVal;
}

} // namespace

bool SPIClass::begin()
{
	if(busAssigned) {
		debug_e("[SPI] Bus already assigned");
		return false;
	}

	busAssigned = true;

	SpiDevice dev;
	dev.init();

	CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX, BIT9);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2); // HSPIQ MISO == GPIO12
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); // HSPID MOSI == GPIO13
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); // CLK		 == GPIO14

	checkSpeed(SPIDefaultSettings.speed);
	prepare(SPIDefaultSettings);

	return true;
}

void SPIClass::end()
{
	GET_DEVICE();

	busAssigned = false;
}

uint32_t SPIClass::transfer32(uint32_t data, uint8_t bits)
{
	GET_DEVICE(0);

	dev.write(data);
	dev.send(bits);
	dev.wait();

	return dev.read();
}

uint8_t SPIClass::read8()
{
	GET_DEVICE(0);

	dev.write(0xff);
	dev.send(8);
	dev.wait();

	return dev.read();
}

void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
	GET_DEVICE();

	for(unsigned i = 0; i < numberBytes; i += SPI_FIFO_SIZE) {
		auto blockLen = std::min(numberBytes - i, SPI_FIFO_SIZE);

		dev.write(&buffer[i], blockLen);
		dev.send(blockLen * 8);
		dev.wait();
		dev.read(&buffer[i], blockLen);
	}
}

void SPIClass::prepare(SPISettings& settings)
{
#ifdef SPI_DEBUG
	debugf("[SPI] prepare()");
	settings.print("settings");
#endif

	GET_DEVICE();

	dev.set_clock(settings.speed);
	dev.set_bit_order(settings.bitOrder);
	dev.set_mode(settings.dataMode);
}
