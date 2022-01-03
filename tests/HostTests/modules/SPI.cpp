#include <HostTests.h>
#include <SPISoft.h>

// #define TEST_SOFTWARE_SPI

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

#ifdef TEST_SOFTWARE_SPI
constexpr uint8_t ss_miso = 12;
constexpr uint8_t ss_mosi = 13;
constexpr uint8_t ss_sck = 14;

SPISoft sspi1(ss_miso, ss_mosi, ss_sck, 0);
#endif

} // namespace

class SpiTest : public TestGroup
{
public:
	SpiTest()
		: TestGroup(_F("SPI")),
#ifdef TEST_SOFTWARE_SPI
		  spi(sspi1)
#else
		  spi(SPI)
#endif
	{
	}

	void execute() override
	{
		// spi.setup(SpiBus::SPI3);
		spi.begin();

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
			spi.beginTransaction(settings);
			spi.transfer32(0x00AA00AA);
			spi.transfer32(0x12345678);
			uint8_t data[]{0x12, 0x34, 0x56, 0x78, 0x9A};
			spi.transfer(data, ARRAY_SIZE(data));
			spi.endTransaction();
			delayMicroseconds(100);
		}

		delayMicroseconds(200);
		for(auto dataMode : {SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3}) {
			settings.bitOrder = MSBFIRST;
			settings.dataMode = dataMode;
			spi.beginTransaction(settings);
			spi.transfer32(0xAA, 13);
			spi.endTransaction();
			delayMicroseconds(100);
		}

		// Leave bus in mode 0
		settings.dataMode = SPI_MODE0;
		spi.beginTransaction(settings);
		spi.endTransaction();

		m_putc('.');

		if(loopCount-- != 0) {
			return;
		}

		timer.stop();
		loopbackTests();
		spi.end();
		complete();
	}

	void loopbackTests()
	{
		if(!spi.loopback(true)) {
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
				settings.bitOrder = bitOrder;
				for(auto bits : {2, 3, 7, 8, 9, 15, 16, 17, 19, 23, 24, 25, 29, 30, 31}) {
					send(0, bits);
					send(0xffffffff, bits);
					send(0xaaaaaaaa, bits);
					send(0x55555555, bits);
					send(0x12345678, bits);
					send(~0x12345678, bits);
				}
			}
		}

		TEST_CASE("Byte sequences")
		{
			DEFINE_FSTR_LOCAL(seq1, "This is a longer sequence but no more than 64 bytes");
			for(auto bitOrder : {MSBFIRST, LSBFIRST}) {
				settings.bitOrder = bitOrder;
				for(unsigned offset = 0; offset < 4; ++offset) {
					// Small packet, fits in FIFO
					send(seq1, offset);

					// Packet larger than FIFO
					String seq2 = seq1;
					seq2 += seq2;
					seq2 += seq2;
					seq2 += seq2;
					seq2 += seq2;
					send(seq2, offset);
				}
			}
		}
	}

	void send(uint32_t outValue, uint8_t bits)
	{
		outValue &= BIT(bits) - 1;
		Serial.print("TX 0x");
		Serial.print(outValue, HEX);
		Serial.print(", ");
		Serial.print(bits);
		Serial.print(" bits, ");
		Serial.print(settings.bitOrder == MSBFIRST ? 'M' : 'L');
		Serial.println("SB first");
		printBin(">", outValue);

		spi.beginTransaction(settings);
		uint32_t inValue = spi.transfer32(outValue, bits);
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

	void send(const String& outData, unsigned startOffset)
	{
		String inData = outData;
		auto bufptr = reinterpret_cast<uint8_t*>(inData.begin() + startOffset);
		auto length = inData.length() - startOffset;

		debug_i("TX %u bytes, startOffset %u, %cSB first", length, startOffset,
				settings.bitOrder == MSBFIRST ? 'M' : 'L');
		spi.beginTransaction(settings);
		spi.transfer(bufptr, length);
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
	SPIBase& spi;
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
