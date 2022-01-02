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
		SPI.begin();

		TEST_CASE("Scope check")
		{
			debug_w("Connect scope and observe bit pattern");
			constexpr unsigned duration{10};
			constexpr unsigned loopInterval{250};
			loopCount = duration * 1000 / loopInterval;
			timer.initializeMs<loopInterval>([&]() {
				for(uint8_t bitMode : {MSBFIRST, LSBFIRST}) {
					settings.bitOrder = bitMode;
					SPI.beginTransaction(settings);
					SPI.transfer32(0x00AA00AA);
					SPI.transfer32(0x12345678);
					SPI.endTransaction();
				}
				m_putc('.');
				if(loopCount-- == 0) {
					this->loopbackTests();
					this->complete();
				}
			});
			timer.start();
			return pending();
		}
	}

	void loopbackTests()
	{
		if(!SPI.loopback(true)) {
			debug_w("WARNING: SPI loopback not supported. Manual connection required.");
			debug_w("ESP8266: Connect MISO (GPIO12/D6) <-> MISO (GPIO13/D7)");
			allowFailure = true;
		}

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
		SPISettings settings;
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

		SPISettings settings;
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
	SPISettings settings{1000000, MSBFIRST, SPI_MODE0};
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
