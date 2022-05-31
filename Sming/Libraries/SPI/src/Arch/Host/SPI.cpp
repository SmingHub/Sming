#include "SPI.h"
#include <FIFO.h>
#include <debug_progmem.h>
#include <stringconversion.h>
#include <Data/BitSet.h>

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
class SpiDevice
{
public:
	void init()
	{
	}

	void deinit()
	{
	}

	uint16_t configure(uint8_t data_bits, uint8_t mode, uint16_t clk)
	{
		return data_bits | mode | clk;
	}

	void set_data_bits(uint16_t cr0val, uint8_t data_bits)
	{
		bits = data_bits;
	}

	bool can_read() const
	{
		return true;
	}

	bool can_write() const
	{
		return true;
	}

	uint16_t read()
	{
		auto c = fifo.dequeue();
		if(ioCallback) {
			ioCallback(c, bits, true);
		}
		return c;
	}

	void write(uint16_t c)
	{
		c &= BIT(bits) - 1;
		if(ioCallback) {
			ioCallback(c, bits, false);
		}
		fifo.enqueue(c);
	}

	uint16_t read_blocking()
	{
		return read();
	}

	void write_blocking(uint16_t c)
	{
		write(c);
	}

	SPIClass::IoCallback ioCallback;

private:
	FIFO<uint16_t, SPI_FIFO_DEPTH> fifo;
	uint8_t bits{0};
};

SpiDevice devices[SOC_SPI_PERIPH_NUM];

SpiDevice& getDevice(SpiBus busId)
{
	return devices[unsigned(busId) - 1];
}

BitSet<uint8_t, SpiBus, SOC_SPI_PERIPH_NUM + 1> busAssigned;

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

} // namespace

SPIClass::SPIClass() : SPIBase({1, 2, 3})
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

	prepare(SPIDefaultSettings);
	return true;
}

void SPIClass::end()
{
	GET_DEVICE();
	dev.deinit();
	busAssigned -= busId;
}

void SPIClass::setDebugIoCallback(IoCallback callback)
{
	GET_DEVICE();
	dev.ioCallback = callback;
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

	cr0val = dev.configure(8, settings.dataMode, 0);

	lsbFirst = (settings.bitOrder == LSBFIRST);
}

bool SPIClass::loopback(bool enable)
{
	(void)enable;
	return true;
}
