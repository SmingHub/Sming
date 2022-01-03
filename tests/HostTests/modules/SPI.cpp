#include <HostTests.h>

#if defined(ARCH_ESP8266) || defined(ARCH_ESP32)

namespace
{
void printBin(const char* tag, uint32_t value)
{
	char buf[40];
	ultoa_wp(value, buf, 2, 32, '0');
	m_puts(tag);
	m_putc(' ');
	m_nputs(buf, 8);
	m_putc(' ');
	m_nputs(buf + 8, 8);
	m_putc(' ');
	m_nputs(buf + 16, 8);
	m_putc(' ');
	m_nputs(buf + 24, 8);
	m_puts("\r\n");
}

} // namespace

class SpiTest : public TestGroup
{
public:
	SpiTest() : TestGroup(_F("SPI"))
	{
	}

	void execute() override
	{
		// SPI.setup(SpiBus::SPI3);
		SPI.begin();

		for(uint8_t mode : {SPI_MODE1, SPI_MODE2, SPI_MODE3}) {
			uint8_t mode_num = ((mode & 0x10) >> 3) | (mode & 0x01);
			debug_i("MODE 0x%02x %u", mode, mode_num);
		}

		TEST_CASE("Bit Patterns")
		{
			debug_w("Connect scope and observe bit pattern");
			constexpr unsigned duration{10};
			constexpr unsigned loopInterval{250};
			loopCount = duration * 1000 / loopInterval;
			timer.initializeMs<loopInterval>([this]() { bitPatterns(); });
			timer.start();
			return pending();
		}
	}

	/*
	 * 6 transactions:
	 *
	 * - MODE0 MSBFIRST       0xAA00AA00    0x12345678    0x12 0x34 0x56 0x78 0x9A
	 * 	   Viewed MSB first:    00 AA 00 AA   12 34 56 78   12 34 56 78 9A
	 *     Viewed LSB first:    00 55 00 55   48 2C 6A 1E   48 2C 6A 1E 59
	 * - MODE0 LSBFIRST       0xAA00AA00    0x12345678    0x12 0x34 0x56 0x78 0x9A
	 *     Viewed MSB first:    55 00 55 00   1E 6A 2C 48   48 2C 6A 1E 59
	 * 	   Viewed LSB first:    AA 00 AA 00   78 56 34 12   12 34 56 78 9A
	 * - MODE0 MSBFIRST 0xAA (13 bits)
	 *     Clock idles LOW, latch on RISING edge
	 * - MODE1 MSBFIRST 0xAA (13 bits)
	 *     Clock idles LOW, latch on FALLING edge
	 * - MODE2 MSBFIRST 0xAA (13 bits)
	 *     Clock idles HIGH, latch on FALLINGg edge
	 * - MODE3 MSBFIRST 0xAA (13 bits)
	 *     Clock idles HIGH, latch on RISING edge
	 */
	void bitPatterns()
	{
		settings.speed = 100000;
		settings.bitOrder = MSBFIRST;
		settings.dataMode = SPI_MODE0;
		for(uint8_t bitOrder : {MSBFIRST, LSBFIRST}) {
			settings.bitOrder = bitOrder;
			SPI.beginTransaction(settings);
			SPI.transfer32(0x00AA00AA);
			SPI.transfer32(0x12345678);
			uint8_t data[]{0x12, 0x34, 0x56, 0x78, 0x9A};
			SPI.transfer(data, ARRAY_SIZE(data));
			SPI.endTransaction();
			delayMicroseconds(100);
		}

		delayMicroseconds(200);
		for(auto dataMode : {SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3}) {
			settings.bitOrder = MSBFIRST;
			settings.dataMode = dataMode;
			SPI.beginTransaction(settings);
			SPI.transfer32(0xAA, 13);
			SPI.endTransaction();
			delayMicroseconds(100);
		}

		// Leave bus in mode 0
		settings.dataMode = SPI_MODE0;
		SPI.beginTransaction(settings);
		SPI.endTransaction();

		m_putc('.');

		if(loopCount-- != 0) {
			return;
		}

		timer.stop();
		loopbackTests();
		SPI.end();
		complete();
	}

	void loopbackTests()
	{
		if(!SPI.loopback(true)) {
			debug_w("WARNING: SPI loopback not supported. Manual connection required.");
			debug_w("ESP8266: Connect MISO (GPIO12/D6) <-> MOSI (GPIO13/D7)");
			allowFailure = true;
		}

		settings.speed = 8000000;
		settings.dataMode = SPI_MODE0;

		TEST_CASE("32-bit values")
		{
			// Note: Single-bit transfers fail on esp32c3... so start at 2
			for(auto bitOrder : {MSBFIRST, LSBFIRST}) {
				for(auto bits : {2, 3, 7, 8, 9, 15, 16, 17, 19, 23, 24, 25, 29, 30, 31}) {
					send(0, bits, bitOrder);
					send(0xffffffff, bits, bitOrder);
					send(0xaaaaaaaa, bits, bitOrder);
					send(0x55555555, bits, bitOrder);
					send(0x12345678, bits, bitOrder);
					send(~0x12345678, bits, bitOrder);
				}
			}
		}

		TEST_CASE("Byte sequences")
		{
			DEFINE_FSTR_LOCAL(seq1, "This is a longer sequence but no more than 64 bytes");
			for(auto bitOrder : {MSBFIRST, LSBFIRST}) {
				for(unsigned offset = 0; offset < 4; ++offset) {
					// Small packet, fits in FIFO
					send(seq1, offset, bitOrder);

					// Packet larger than FIFO
					String seq2 = seq1;
					seq2 += seq2;
					seq2 += seq2;
					seq2 += seq2;
					seq2 += seq2;
					send(seq2, offset, bitOrder);
				}
			}
		}
	}

	void send(uint32_t outValue, uint8_t bits, uint8_t bitOrder)
	{
		settings.bitOrder = bitOrder;
		SPI.beginTransaction(settings);
		outValue &= BIT(bits) - 1;
		Serial.print("TX 0x");
		Serial.print(outValue, HEX);
		Serial.print(", ");
		Serial.print(bits);
		Serial.print(" bits, ");
		Serial.print(bitOrder == MSBFIRST ? 'M' : 'L');
		Serial.println("SB first");
		printBin(">", outValue);
		uint32_t inValue = SPI.transfer32(outValue, bits);
		if(inValue != outValue) {
			printBin("<", inValue);
			Serial.print("RX 0x");
			Serial.println(inValue, HEX);
			if(allowFailure) {
				fail(__PRETTY_FUNCTION__);
				return;
			}
		}

		REQUIRE(inValue == outValue);
	}

	void send(const String& outData, unsigned startOffset, uint8_t bitOrder)
	{
		String inData = outData;
		auto bufptr = reinterpret_cast<uint8_t*>(inData.begin() + startOffset);
		auto length = inData.length() - startOffset;

		settings.bitOrder = bitOrder;
		SPI.beginTransaction(settings);
		debug_i("TX %u bytes, startOffset %u, %cSB first", length, startOffset, bitOrder == MSBFIRST ? 'M' : 'L');
		SPI.transfer(bufptr, length);
		auto outptr = outData.c_str() + startOffset;
		if(memcmp(outptr, bufptr, length) != 0) {
			length = std::min(length, 64U);
			m_printHex(">", outptr, length);
			m_printHex("<", bufptr, length);
			if(allowFailure) {
				fail(__PRETTY_FUNCTION__);
			} else {
				TEST_ASSERT(false);
			}
		}
	}

private:
	Timer timer;
	SPISettings settings;
	unsigned loopCount{0};
	bool allowFailure{false};
};

#endif

void REGISTER_TEST(SPI)
{
#if defined(ARCH_ESP8266) || defined(ARCH_ESP32)
	registerGroup<SpiTest>();
#endif
}
