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

// define the static singleton
SPIClass SPI;

using SpiDevice = volatile spi_dev_t;

namespace
{
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

/**
 * @brief Wait until SPI has finished any current transaction
 */
__forceinline void spi_wait(SpiDevice& dev)
{
	while(dev.cmd.usr) {
		//
	}
}

/**
 * @brief Initiate an SPI user transaction
 */
__forceinline void spi_send(SpiDevice& dev)
{
	dev.cmd.usr = true;
}

/**
 * @brief	Configure SPI mode parameters for clock edge and clock polarity.
 * @param	device
 * @param	mode
 *
 * 	 		Mode		Clock Polarity (CPOL)	Clock Phase (CPHA)
 *			SPI_MODE0	0						0
 *			SPI_MODE1	0						1
 *			SPI_MODE2	1						0
 *			SPI_MODE3	1						1
 */
void spi_mode(SpiDevice& dev, uint8_t mode)
{
	spi_ll_master_set_mode(&dev, mode);

#ifdef SPI_DEBUG
	debug_i("[SPI] spi_mode(mode %x) cpha %X, cpol %X)", mode, mode & 0x0F, mode & 0xF0);
#endif
}

/**
 * @brief	Setup the byte order for shifting data out of buffer
 * @param	MSBFIRST	1
 * 	 		Data is sent out starting with Bit31 and down to Bit0
 * 	 		LSBFIRST	0
 * 	 		Data is sent out starting with the lowest BYTE, from MSB to LSB
 * 			0xABCDEFGH would be sent as 0xGHEFCDAB
 */
void spi_byte_order(SpiDevice& dev, uint8_t byte_order)
{
#ifdef SPI_DEBUG
	debug_i("[SPI] spi_byte_order(byte_order %u)", byte_order);
#endif

// No HAL definition for this
#if SOC_ESP32 || SOC_ESP32S2
	dev.user.rd_byte_order = (byte_order == MSBFIRST);
	dev.user.wr_byte_order = (byte_order == MSBFIRST);
#else
// No definition in datasheet for esp32-c3, perhaps it's just missing?
#warning "SPI byte order unsupported"
#endif
}

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
	debug_i("[SPI] calculateSpeed(freq %u, pre %u, div %u)", freq, pre, div);
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

	debug_i("[SPI] APB freq = %u, pre = %u, div = %u, target freq = %u, actual = %u", apbFreq, prediv.prescale,
			prediv.divisor, speed.frequency, prediv.freq);
}

uint32_t getApbFrequency()
{
	constexpr uint32_t DIV_MHZ{1000000};
	rtc_cpu_freq_config_t conf;
	rtc_clk_cpu_freq_get_config(&conf);
	return (conf.freq_mhz >= 80) ? (80 * DIV_MHZ) : ((conf.source_freq_mhz * DIV_MHZ) / conf.div);
}

void spi_set_clock(SpiDevice& dev, SPISpeed& speed)
{
	// Clock register value is never 0, so indicates it hasn't been calculated
	if(speed.regVal == 0) {
		checkSpeed(speed, getApbFrequency());
	} else {
#ifdef SPI_DEBUG
		debug_i("[SPI] spi_set_clock(%u)", speed.frequency);
#endif
	}

	dev.clock.val = speed.regVal;
}

const spi_signal_conn_t& getBusInfo(SpiBus busId)
{
	return spi_periph_signal[unsigned(busId) - 1];
}

SpiDevice& getDevice(SpiBus busId)
{
	return *getBusInfo(busId).hw;
}

struct BusState {
	bool assigned : 1;
};

BusState busState[SOC_SPI_PERIPH_NUM];

BusState& getBusState(SpiBus busId)
{
	return busState[unsigned(busId) - 1];
}

} // namespace

bool SPIClass::setup(SpiBus busId, SpiPins pins)
{
	if(busId < SpiBus::MIN || busId > SpiBus::MAX) {
		debug_e("[SPI] Invalid bus");
		return false;
	}

	if(getBusState(busId).assigned) {
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

	auto& state = getBusState(busId);
	if(state.assigned) {
		debug_e("[SPI] Bus #%u already assigned", busId);
		return false;
	}

	auto& bus = getBusInfo(busId);
	periph_ll_enable_clk_clear_rst(bus.module);

	auto& dev = getDevice(busId);

	// Initialise bus
	spi_ll_master_init(&dev);
	spi_ll_clear_int_stat(&dev);

	//
	spi_ll_enable_mosi(&dev, true);
	spi_ll_enable_miso(&dev, true);
	spi_ll_set_half_duplex(&dev, false);

	spi_ll_set_dummy(&dev, 0);
	spi_ll_set_command_bitlen(&dev, 0);
	spi_ll_set_addr_bitlen(&dev, 0);

	// Not using any auto. chip selects
	spi_ll_master_select_cs(&dev, -1);

	auto& defPins = defaultPins[unsigned(busId) - 1];

	// Clock pin
	if(pins.sck == SPI_PIN_DEFAULT) {
		pins.sck = defPins.sck;
	}
	pinMode(pins.sck, OUTPUT);
	gpio_matrix_out(pins.sck, bus.spiclk_out, false, false);

	// MISO
	if(pins.miso == SPI_PIN_DEFAULT) {
		pins.miso = defPins.miso;
	}
	pinMode(pins.miso, INPUT);
	gpio_matrix_in(pins.miso, bus.spiq_in, false);

	// MOSI
	if(pins.mosi == SPI_PIN_DEFAULT) {
		pins.mosi = defPins.mosi;
	}
	pinMode(pins.mosi, OUTPUT);
	gpio_matrix_out(pins.mosi, bus.spid_out, false, false);

	debug_i("[SPI] SCK = %u, MISO = %u, MOSI = %u", pins.sck, pins.miso, pins.mosi);

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

	state.assigned = true;
	return true;
}

void SPIClass::end()
{
	if(busId < SpiBus::MIN || busId > SpiBus::MAX) {
		return;
	}

	auto& state = getBusState(busId);
	if(!state.assigned) {
		return;
	}

	auto& info = getBusInfo(busId);
	periph_ll_disable_clk_set_rst(info.module);

	state.assigned = false;
}

uint32_t SPIClass::transfer32(uint32_t data, uint8_t bits)
{
	auto& dev = getDevice(busId);

	spi_wait(dev);

	spi_ll_set_mosi_bitlen(&dev, bits);
	spi_ll_set_miso_bitlen(&dev, bits);

	// copy data to W0
#if SOC_ESP32 || SOC_ESP32S2
	if(dev.user.wr_byte_order) {
		dev.data_buf[0] = data << (32 - bits);
	} else
#endif
	{
		dev.data_buf[0] = data;
	}

	spi_send(dev);
	spi_wait(dev);

	auto res = dev.data_buf[0];
#if SOC_ESP32 || SOC_ESP32S2
	if(dev.user.rd_byte_order) {
		res >>= (32 - bits);
	}
#endif
	return res;
}

uint8_t SPIClass::read8()
{
	auto& dev = getDevice(busId);

	spi_wait(dev);

	dev.data_buf[0] = 0x00;

	spi_send(dev);
	spi_wait(dev);

	auto res = dev.data_buf[0];
#if SOC_ESP32 || SOC_ESP32S2
	if(dev.user.rd_byte_order) {
		res >>= 24;
	}
#endif
	return res;
}

void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
	constexpr size_t BLOCKSIZE{64}; // the max length of the ESP SPI_W0 registers

	auto& dev = getDevice(busId);

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
		debug_i("[SPI] Write/Read Block %u total %u bytes", total - blocks, bufLength);
#endif

		spi_wait(dev);

		// setup bit length
		auto num_bits = bufLength * 8;
		spi_ll_set_mosi_bitlen(&dev, num_bits);
		spi_ll_set_miso_bitlen(&dev, num_bits);

		// copy the registers starting from last index position
		if(IS_ALIGNED(buffer)) {
			memcpy((void*)dev.data_buf, &buffer[bufIndx], ALIGNUP4(bufLength));
		} else {
			uint32_t wordBuffer[BLOCKSIZE / 4];
			memcpy(wordBuffer, &buffer[bufIndx], bufLength);
			memcpy((void*)dev.data_buf, wordBuffer, ALIGNUP4(bufLength));
		}

		spi_send(dev);
		spi_wait(dev);

		// copy the registers starting from last index position
		memcpy(&buffer[bufIndx], (void*)dev.data_buf, bufLength);

		bufIndx += bufLength;
	}
}

void SPIClass::prepare(SPISettings& settings)
{
#ifdef SPI_DEBUG
	debug_i("[SPI] prepare()");
	settings.print("settings");
#endif

	auto& dev = getDevice(busId);

	spi_set_clock(dev, settings.speed);
	spi_byte_order(dev, settings.byteOrder);
	spi_mode(dev, settings.dataMode);
}
