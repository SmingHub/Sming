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
#undef FLAG_ATTR
#define FLAG_ATTR(TYPE)
#define typeof decltype
#include <soc/spi_periph.h>
#include <hal/spi_ll.h>
#include <hal/clk_gate_ll.h>
#include <soc/rtc.h>
#include <Data/BitSet.h>

#define GET_DEVICE(err)                                                                                                \
	if(!busAssigned[busId]) {                                                                                          \
		debug_e("[SPI] Not Ready");                                                                                    \
		return err;                                                                                                    \
	}                                                                                                                  \
	SpiDevice dev{busId};

SPIClass SPI;

namespace
{
constexpr size_t SPI_FIFO_SIZE{64};

const SpiPins defaultPins[] = {
	{6, 7, 8, SPI_PIN_DEFAULT},
	{14, 12, 13, SPI_PIN_DEFAULT},
	{18, 19, 23, SPI_PIN_DEFAULT},
};

// Used internally to calculate optimum SPI speed
struct SpiPreDiv {
	unsigned freq;
	unsigned prescale;
	unsigned divisor;
};

struct SpiDevice {
	const spi_signal_conn_t& info;

	SpiDevice(SpiBus busId) : info(spi_periph_signal[unsigned(busId) - 1])
	{
	}

	void init()
	{
		periph_ll_enable_clk_clear_rst(info.module);

		// Initialise bus
		spi_ll_master_init(info.hw);
		spi_ll_clear_int_stat(info.hw);

		//
		spi_ll_enable_mosi(info.hw, true);
		spi_ll_enable_miso(info.hw, false);
		spi_ll_set_half_duplex(info.hw, false);

		spi_ll_set_dummy(info.hw, 0);
		spi_ll_set_command_bitlen(info.hw, 0);
		spi_ll_set_addr_bitlen(info.hw, 0);

		// Not using any auto. chip selects
		spi_ll_master_select_cs(info.hw, -1);
	}

	void deinit()
	{
		periph_ll_disable_clk_set_rst(info.module);
	}

	void set_clock(SPISpeed& speed);

	/**
	 * @brief Wait until SPI has finished any current transaction
	 */
	void wait()
	{
		while(info.hw->cmd.usr) {
			//
		}
	}

	/**
	 * @brief Initiate an SPI user transaction
	 */
	void send(unsigned num_bits)
	{
		spi_ll_set_mosi_bitlen(info.hw, num_bits);
		info.hw->cmd.usr = true;
	}

	void set_mode(uint8_t mode)
	{
#ifdef SPI_DEBUG
		debugf("[SPI] set_mode(mode %x) cpha %X, cpol %X)", mode, mode & 0x0F, mode & 0xF0);
#endif
		uint8_t mode_num = ((mode & 0x10) >> 3) | (mode & 0x01);
		spi_ll_master_set_mode(info.hw, mode_num);
	}

	void set_bit_order(uint8_t bit_order)
	{
#ifdef SPI_DEBUG
		debugf("[SPI] set_bit_order(bit_order %u)", bit_order);
#endif

		spi_ll_set_rx_lsbfirst(info.hw, bit_order != MSBFIRST);
		spi_ll_set_tx_lsbfirst(info.hw, bit_order != MSBFIRST);
	}

	uint32_t read()
	{
		return info.hw->data_buf[0];
	}

	void write(uint32_t value)
	{
		info.hw->data_buf[0] = value;
	}

	void read(void* buffer, size_t length)
	{
		if(IS_ALIGNED(buffer) && IS_ALIGNED(length)) {
			memcpy(buffer, (void*)info.hw->data_buf, length);
		} else {
			uint32_t wordBuffer[SPI_FIFO_SIZE];
			memcpy(wordBuffer, (void*)info.hw->data_buf, ALIGNUP4(length));
			memcpy(buffer, wordBuffer, length);
		}
	}

	void write(const void* buffer, size_t length)
	{
		if(IS_ALIGNED(buffer)) {
			memcpy((void*)info.hw->data_buf, buffer, ALIGNUP4(length));
		} else {
			uint32_t wordBuffer[SPI_FIFO_SIZE];
			memcpy(wordBuffer, buffer, length);
			memcpy((void*)info.hw->data_buf, wordBuffer, ALIGNUP4(length));
		}
	}
};

/**
 * @brief Calculate the closest prescale value for a given frequency and clock-divider
 * @param apbFreq APB CPU frequency, in Hz
 * @param freq target SPI bus frequency, in Hz
 * @param div divisor value to use
 * @retval SpiPreDiv contains resulting frequency, prescaler and divisor values
 */
SpiPreDiv calculateSpeed(unsigned apbFreq, unsigned freq, unsigned div)
{
	SpiPreDiv prediv;
	unsigned pre = apbFreq / (freq * div);
	if(pre == 0) {
		pre = 1;
	}
	unsigned n = pre * div;
	while(true) {
		prediv.freq = apbFreq / n;
		if(prediv.freq <= freq) {
			break;
		}
		++pre;
		n += div;
	}
	prediv.prescale = pre;
	prediv.divisor = div;

#ifdef SPI_DEBUG
	debugf("[SPI] calculateSpeed(freq %u, pre %u, div %u)", freq, pre, div);
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
 */
void checkSpeed(SPISpeed& speed, unsigned apbFreq)
{
	SpiPreDiv prediv;

	// If we're not running at max then need to determine appropriate prescale values
	if(speed.frequency >= apbFreq) {
		// Use maximum speed
		prediv.freq = apbFreq;
		prediv.prescale = 0;
		prediv.divisor = 0;
		speed.regVal = SPI_CLK_EQU_SYSCLK;
	} else {
		prediv = calculateSpeed(apbFreq, speed.frequency, 2);
		if(prediv.freq != speed.frequency) {
			// Use whichever divisor gives the highest frequency
			SpiPreDiv pd3 = calculateSpeed(apbFreq, speed.frequency, 3);
			SpiPreDiv pd5 = calculateSpeed(apbFreq, speed.frequency, 5);
			if(pd3.freq > prediv.freq || pd5.freq > prediv.freq) {
				prediv = (pd3.freq > pd5.freq) ? pd3 : pd5;
			}
		}

		// We have prescale and divisor values, now get regVal so we don't need to do this every time prepare() is called
		decltype(spi_dev_t::clock) reg{{
			.clkcnt_l = 0,
			.clkcnt_h = prediv.divisor / 2,
			.clkcnt_n = prediv.divisor - 1,
			.clkdiv_pre = prediv.prescale - 1,
		}};
		speed.regVal = reg.val;
	}

#ifdef SPI_DEBUG
	debugf("[SPI] APB freq = %u, pre = %u, div = %u, target freq = %u, actual = %u", apbFreq, prediv.prescale,
		   prediv.divisor, speed.frequency, prediv.freq);
#endif
}

uint32_t getApbFrequency()
{
	constexpr uint32_t DIV_MHZ{1000000};
	rtc_cpu_freq_config_t conf;
	rtc_clk_cpu_freq_get_config(&conf);
	return (conf.freq_mhz >= 80) ? (80 * DIV_MHZ) : ((conf.source_freq_mhz * DIV_MHZ) / conf.div);
}

void SpiDevice::set_clock(SPISpeed& speed)
{
	// Clock register value is never 0, so indicates it hasn't been calculated
	if(speed.regVal == 0) {
		checkSpeed(speed, getApbFrequency());
	} else {
#ifdef SPI_DEBUG
		debugf("[SPI] spi_set_clock(%u)", speed.frequency);
#endif
	}

	info.hw->clock.val = speed.regVal;
}

BitSet<uint8_t, SpiBus, SOC_SPI_PERIPH_NUM + 1> busAssigned;

} // namespace

bool SPIClass::setup(SpiBus busId, SpiPins pins)
{
	if(busId < SpiBus::MIN || busId > SpiBus::MAX) {
		debug_e("[SPI] Invalid bus");
		return false;
	}

	if(busAssigned[busId]) {
		debug_e("[SPI] Bus #%u already assigned", busId);
		return false;
	}

	this->busId = busId;
	this->pins = pins;
	return true;
}

bool SPIClass::begin()
{
	if(busId < SpiBus::MIN || busId > SpiBus::MAX) {
		debug_e("[SPI] Invalid bus");
		return false;
	}

	if(busAssigned[busId]) {
		debug_e("[SPI] Bus #%u already assigned", busId);
		return false;
	}

	busAssigned += busId;

	SpiDevice dev(busId);
	dev.init();

	auto& defPins = defaultPins[unsigned(busId) - 1];

	// Clock pin
	if(pins.sck == SPI_PIN_DEFAULT) {
		pins.sck = defPins.sck;
	}
	pinMode(pins.sck, OUTPUT);
	gpio_matrix_out(pins.sck, dev.info.spiclk_out, false, false);

	// MISO
	if(pins.miso == SPI_PIN_DEFAULT) {
		pins.miso = defPins.miso;
	}
	pinMode(pins.miso, INPUT);
	gpio_matrix_in(pins.miso, dev.info.spiq_in, false);

	// MOSI
	if(pins.mosi == SPI_PIN_DEFAULT) {
		pins.mosi = defPins.mosi;
	}
	pinMode(pins.mosi, OUTPUT);
	gpio_matrix_out(pins.mosi, dev.info.spid_out, false, false);

#ifdef SPI_DEBUG
	debugf("[SPI] SCK = %u, MISO = %u, MOSI = %u", pins.sck, pins.miso, pins.mosi);
#endif

	// Clock

	// TODO: Framework needs to consider how to manage system clock changes
	//	addApbChangeCallback(this, [](void* arg, apb_change_ev_t ev_type, uint32_t old_apb, uint32_t new_apb) {
	//		auto cls = static_cast<SPIClass*>(arg);
	//		auto& dev = cls->bus.dev;
	//		if(ev_type == APB_BEFORE_CHANGE) {
	//			while(dev.cmd.usr) {
	//				//
	//			}
	//		} else {
	//			setClock(cls->speed, new_apb);
	//		}
	//	});

	checkSpeed(SPIDefaultSettings.speed, getApbFrequency());
	prepare(SPIDefaultSettings);

	return true;
}

void SPIClass::end()
{
	GET_DEVICE();

	dev.deinit();
	busAssigned -= busId;
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
