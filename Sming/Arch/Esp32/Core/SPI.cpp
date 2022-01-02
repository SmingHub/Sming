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

#define DEFPIN(sig, bus) SPI##bus##_IOMUX_PIN_NUM_##sig

const SpiPins defaultPins[SOC_SPI_PERIPH_NUM] = {
	{DEFPIN(CLK, ), DEFPIN(MISO, ), DEFPIN(MOSI, ), SPI_PIN_DEFAULT},
	{DEFPIN(CLK, 2), DEFPIN(MISO, 2), DEFPIN(MOSI, 2), SPI_PIN_DEFAULT},
#if SOC_SPI_PERIPH_NUM >= 3
	{DEFPIN(CLK, 3), DEFPIN(MISO, 3), DEFPIN(MOSI, 3), SPI_PIN_DEFAULT},
#endif
};

struct SpiDevice {
	const spi_signal_conn_t& info;

	explicit SpiDevice(SpiBus busId) : info(spi_periph_signal[unsigned(busId) - 1])
	{
	}

	void init()
	{
		periph_ll_enable_clk_clear_rst(info.module);

		// Initialise bus
		spi_ll_master_init(info.hw);
		spi_ll_disable_int(info.hw);
		spi_ll_set_mosi_delay(info.hw, 0, 0);

		// Configure full duplex - bi-directdional transfer in MOSI phase
		spi_ll_set_half_duplex(info.hw, false);
		spi_ll_enable_mosi(info.hw, true);
		spi_ll_enable_miso(info.hw, true);

		// We only use data phase, disable the others
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
		spi_ll_set_miso_bitlen(info.hw, num_bits);
		spi_ll_master_user_start(info.hw);
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

/** @brief Check speed settings and perform any pre-calculation required
 *  @param speed IN: requested bus frequency, OUT: Modified settings with prescale values
 */
void checkSpeed(SPISpeed& speed)
{
	constexpr int duty_cycle{127};
	spi_ll_clock_val_t clock_reg;
	unsigned actual_freq = spi_ll_master_cal_clock(SPI_LL_PERIPH_CLK_FREQ, speed.frequency, duty_cycle, &clock_reg);
	speed.regVal = clock_reg;

	// #ifdef SPI_DEBUG
	debugf("[SPI] target freq = %u, actual = %u", speed.frequency, actual_freq);
	// #endif
}

void SpiDevice::set_clock(SPISpeed& speed)
{
	// Clock register value is never 0, so indicates it hasn't been calculated
	if(speed.regVal == 0) {
		checkSpeed(speed);
	} else {
#ifdef SPI_DEBUG
		debugf("[SPI] spi_set_clock(%u)", speed.frequency);
#endif
	}

	spi_ll_master_set_clock_by_reg(info.hw, &speed.regVal);
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

	// Configure pins
	auto& defPins = defaultPins[unsigned(busId) - 1];
	if(pins.sck == SPI_PIN_DEFAULT) {
		pins.sck = defPins.sck;
	}
	if(pins.miso == SPI_PIN_DEFAULT) {
		pins.miso = defPins.miso;
	}
	if(pins.mosi == SPI_PIN_DEFAULT) {
		pins.mosi = defPins.mosi;
	}

	bool useIomux = (pins.sck == dev.info.spiclk_iomux_pin && pins.miso == dev.info.spiq_iomux_pin &&
					 pins.mosi == dev.info.spid_iomux_pin);
	if(useIomux) {
		// Use IO Mux
		gpio_iomux_in(pins.sck, dev.info.spiclk_in);
		gpio_iomux_out(pins.sck, dev.info.func, false);

		gpio_iomux_in(pins.miso, dev.info.spiq_in);
		gpio_iomux_out(pins.miso, dev.info.func, false);

		gpio_iomux_in(pins.mosi, dev.info.spid_in);
		gpio_iomux_out(pins.mosi, dev.info.func, false);
	} else {
		// Use GPIO Mux
		pinMode(pins.sck, OUTPUT);
		gpio_matrix_out(pins.sck, dev.info.spiclk_out, false, false);

		pinMode(pins.miso, INPUT);
		gpio_matrix_in(pins.miso, dev.info.spiq_in, false);

		pinMode(pins.mosi, OUTPUT);
		gpio_matrix_out(pins.mosi, dev.info.spid_out, false, false);
	}

	debug_i("[SPI] Bus #%u using %sIO MUX: SCK %u, MISO %u, MOSI %u", busId, useIomux ? "" : "GP", pins.sck, pins.miso,
			pins.mosi);

	checkSpeed(SPIDefaultSettings.speed);
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

	if(bits != 8 && !lsbFirst) {
		data = __builtin_bswap32(data << (32 - bits));
	}
	dev.write(data);
	dev.send(bits);
	dev.wait();

	data = dev.read();
	if(bits != 8 && !lsbFirst) {
		data = __builtin_bswap32(data) >> (32 - bits);
	}
	return data;
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
	lsbFirst = (settings.bitOrder != MSBFIRST);
	dev.set_mode(settings.dataMode);
}

bool SPIClass::loopback(bool enable)
{
	GET_DEVICE(false);
	gpio_matrix_in(enable ? pins.mosi : pins.miso, dev.info.spiq_in, false);
	return true;
}
