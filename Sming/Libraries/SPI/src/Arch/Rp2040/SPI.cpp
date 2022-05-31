/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SPI.cpp
 *
 ****/

#include "SPI.h"
#include <hardware/structs/spi.h>
#include <hardware/clocks.h>
#include <hardware/address_mapped.h>
#include <hardware/resets.h>
#include <hardware/gpio.h>
#include <Data/BitSet.h>
#include <debug_progmem.h>

#define SPI_FIFO_DEPTH 8

#define GET_DEVICE(err)                                                                                                \
	if(!busAssigned[busId]) {                                                                                          \
		debug_e("[SPI] Not Ready");                                                                                    \
		return err;                                                                                                    \
	}                                                                                                                  \
	auto& dev = getDevice(busId);

SPIClass SPI;

namespace
{
const SpiPins defaultPins[SOC_SPI_PERIPH_NUM] = {
	{.sck = PICO_DEFAULT_SPI_SCK_PIN, .miso = PICO_DEFAULT_SPI_RX_PIN, .mosi = PICO_DEFAULT_SPI_TX_PIN},
	{.sck = 10, .miso = 12, .mosi = 11},
};

//
union ClockReg {
	struct {
		uint8_t prescale;
		uint8_t postdiv;
	};
	uint16_t val;
};

// Used internally to calculate optimum SPI speed
struct SpiPreDiv {
	SpiPreDiv(unsigned freq, uint8_t prescale, uint8_t postdiv) : freq(freq), reg{prescale, postdiv}
	{
	}

	unsigned freq;
	ClockReg reg;
};

struct SpiDevice {
	spi_hw_t* const hw;

	void reset()
	{
		reset_block(hw == spi0_hw ? RESETS_RESET_SPI0_BITS : RESETS_RESET_SPI1_BITS);
	}

	void unreset()
	{
		unreset_block_wait(hw == spi0_hw ? RESETS_RESET_SPI0_BITS : RESETS_RESET_SPI1_BITS);
	}

	void init()
	{
		reset();
		hw->cr1 = 0;
		unreset();

		// Enable the SPI
		hw->cr1 = SPI_SSPCR1_SSE_BITS;
	}

	void deinit()
	{
		hw->cr1 = 0;
		hw->dmacr = 0;
		reset();
	}

	union SSPCR0 {
		struct {
			uint32_t dss : 4; ///< Data Size Select
			uint32_t frf : 2; ///< Frame Format
			uint32_t spo : 1; ///< SSPCLKOUT polarity
			uint32_t sph : 1; ///< SSPCLKOUT phase
			uint32_t scr : 8; ///< Serial clock rate
		};
		uint32_t val;
	};

	enum FrameFormat {
		FRAME_FORMAT_MOTOROLA = 0,
		FRAME_FORMAT_TI = 1,
		FRAME_FORMAT_MICROWIRE = 2,
		FRAME_FORMAT_UNDEFINED = 3,
	};

	uint16_t configure(uint8_t data_bits, uint8_t mode, ClockReg clk)
	{
		hw->cpsr = clk.prescale;
		SSPCR0 cr0{{
			.dss = uint8_t(data_bits - 1),
			.frf = FRAME_FORMAT_MOTOROLA,
			.spo = (mode & 0xF0) ? 1U : 0U,
			.sph = (mode & 0x0F) ? 1U : 0U,
			.scr = uint8_t(clk.postdiv - 1),
		}};
		hw->cr0 = cr0.val;
		return cr0.val;
	}

	void loopback(bool enable)
	{
		hw_write_masked(&hw->cr1, enable << SPI_SSPCR1_LBM_LSB, SPI_SSPCR1_LBM_BITS);
	}

	void set_data_bits(uint16_t cr0val, uint8_t data_bits)
	{
		SSPCR0 cr0{.val = cr0val};
		cr0.dss = data_bits - 1;
		hw->cr0 = cr0.val;
	}

	bool is_busy() const
	{
		return hw->sr & SPI_SSPSR_BSY_BITS;
	}

	bool can_read() const
	{
		return hw->sr & SPI_SSPSR_RNE_BITS;
	}

	bool can_write() const
	{
		return hw->sr & SPI_SSPSR_TNF_BITS;
	}

	uint16_t read()
	{
		return hw->dr;
	}

	void write(uint16_t c)
	{
		hw->dr = c;
	}

	uint16_t read_blocking()
	{
		while(!can_read()) {
		}
		return read();
	}

	void write_blocking(uint16_t c)
	{
		while(!can_write()) {
		}
		write(c);
	}
};

SpiDevice devices[SOC_SPI_PERIPH_NUM] = {
	{spi0_hw},
	{spi1_hw},
};

SpiDevice& getDevice(SpiBus busId)
{
	return devices[unsigned(busId) - 1];
}

BitSet<uint8_t, SpiBus, SOC_SPI_PERIPH_NUM + 1> busAssigned;

// Cortex M0+ doesn't support the rbit instruction
// __forceinline uint32_t reverseBits(uint32_t value)
// {
// 	uint32_t result;
// 	__asm__("rbit %0, %1" : "=r"(result) : "r"(value));
// 	return result;
// }

uint8_t reverseBits(uint8_t n)
{
	static constexpr uint8_t rev_nybble[16]{
		0b0000, 0b1000, 0b0100, 0b1100, 0b0010, 0b1010, 0b0110, 0b1110,
		0b0001, 0b1001, 0b0101, 0b1101, 0b0011, 0b1011, 0b0111, 0b1111,
	};
	return (rev_nybble[n & 0x0f] << 4) | rev_nybble[n >> 4];
}

uint16_t reverseBits(uint16_t n)
{
	return (reverseBits(uint8_t(n)) << 8) | reverseBits(uint8_t(n >> 8));
}

void reverseBits(uint8_t* buffer, size_t length)
{
	while(length--) {
		*buffer = reverseBits(*buffer);
		++buffer;
	}
}

SpiPreDiv calculateSpeed(unsigned baudrate)
{
	auto freq_in = clock_get_hz(clk_peri);
	if(baudrate > freq_in) {
		return SpiPreDiv(freq_in, 2, 1);
	}

	/*
	 * Find smallest prescale value which puts output frequency in range of post-divide.
	 * Prescale is an even number from 2 to 254 inclusive.
	 */
	unsigned prescale;
	for(prescale = 2; prescale <= 254; prescale += 2) {
		if(freq_in < (prescale + 2) * 256 * (uint64_t)baudrate)
			break;
	}
	if(prescale > 254) {
		// Frequency too low
		prescale = 254;
	}

	/*
	 * Find largest post-divide which makes output <= baudrate.
	 * Post-divide is an integer in the range 1 to 256 inclusive.
	 */
	unsigned postdiv;
	for(postdiv = 256; postdiv > 1; --postdiv) {
		if(freq_in / (prescale * (postdiv - 1)) > baudrate)
			break;
	}

	// Return actual frequency and corresponding settings
	return SpiPreDiv(freq_in / (prescale * postdiv), prescale, postdiv);
}

} // namespace

SPIClass::SPIClass() : SPIBase(defaultPins[unsigned(SpiBus::DEFAULT) - 1])
{
}

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
	mPins = pins;
	return true;
}

bool SPIClass::begin()
{
	if(busAssigned[busId]) {
		debug_e("[SPI] Bus #%u already assigned", busId);
		return false;
	}

	busAssigned += busId;

	auto& dev = getDevice(busId);
	dev.init();

	assignDefaultPins(defaultPins[unsigned(busId) - 1]);

	gpio_set_function(pins.sck, GPIO_FUNC_SPI);
	gpio_set_function(pins.miso, GPIO_FUNC_SPI);
	gpio_set_function(pins.mosi, GPIO_FUNC_SPI);
	gpio_pull_up(pins.miso);

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
	if(bits < 4) {
		debug_e("[SPI] Minimum 4 bit transfers");
		return 0;
	}

	GET_DEVICE(0);

	auto writeRead = [&](uint16_t data, uint8_t bits) -> uint16_t {
		dev.set_data_bits(cr0val, bits);
		if(lsbFirst) {
			data = reverseBits(data);
			data >>= 16 - bits;
		}
		dev.write(data);
		data = dev.read_blocking();
		if(lsbFirst) {
			data <<= 16 - bits;
			data = reverseBits(data);
		}
		return data;
	};

	/*
	 * 0-3: Not supported
	 * 4-16: One transfer
	 * 17-24: Two transfers, first is 8 bits
	 * 25-32: Two transfers, first is 16 bits
	 */
	uint32_t res;
	if(bits <= 16) {
		res = writeRead(data, bits);
	} else {
		uint8_t n = (bits <= 24) ? 8 : 16;
		if(lsbFirst) {
			res = writeRead(data, n);
			res |= writeRead(data >> n, bits - n) << n;
		} else {
			res = writeRead(data >> n, bits - n) << n;
			res |= writeRead(data, n);
		}
	}

	return res;
}

uint8_t SPIClass::read8()
{
	return transfer32(0xff, 8);
}

void SPIClass::transfer(uint8_t* buffer, size_t numberBytes)
{
	GET_DEVICE();

	if(lsbFirst) {
		reverseBits(buffer, numberBytes);
	}

	size_t rx_remaining = numberBytes;
	size_t tx_remaining = numberBytes;
	const uint8_t* src = buffer;
	uint8_t* dst = buffer;

	dev.set_data_bits(cr0val, 8);
	while(rx_remaining + tx_remaining != 0) {
		/*
		 * Never have more transfers in flight than will fit into the RX FIFO,
		 * or FIFO will overflow if this code is heavily interrupted.
		 */
		while(tx_remaining != 0 && rx_remaining < tx_remaining + SPI_FIFO_DEPTH && dev.can_write()) {
			dev.write(*src++);
			--tx_remaining;
		}
		if(rx_remaining != 0 && dev.can_read()) {
			*dst++ = dev.read();
			--rx_remaining;
		}
	}

	if(lsbFirst) {
		reverseBits(buffer, numberBytes);
	}
}

void SPIClass::prepare(SPISettings& settings)
{
	GET_DEVICE();

#ifdef SPI_DEBUG
	debug_i("[SPI] prepare()");
	settings.print("settings");
#endif

	// Clock register value is never 0, so indicates it hasn't been calculated
	auto& speed = settings.speed;
	if(speed.regVal == 0) {
		auto prediv = calculateSpeed(speed.frequency);
		speed.regVal = prediv.reg.val;
		debug_i("[SPI] pre = %u, div = %u, target freq = %u, actual = %u", prediv.reg.prescale, prediv.reg.postdiv,
				speed.frequency, prediv.freq);
	}
	ClockReg clk;
	clk.val = settings.speed.regVal;

	cr0val = dev.configure(8, settings.dataMode, clk);

	lsbFirst = (settings.bitOrder == LSBFIRST);
}

bool SPIClass::loopback(bool enable)
{
	GET_DEVICE(false);
	dev.loopback(enable);
	return true;
}
