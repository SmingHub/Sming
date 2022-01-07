#include <SmingTest.h>
#include <Services/Profiling/MinMaxTimes.h>
#include <SPISoft.h>

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

#if SPISOFT_ENABLE

SPISoft spi(0);

#else

SPIBase& spi = SPI;

#endif

#if CPU_FAST
using CycleTimer = CpuCycleTimerFast;
#else
using CycleTimer = CpuCycleTimer;
#endif

} // namespace

class SpiTest : public TestGroup
{
public:
	SpiTest() : TestGroup(F("SPI")), cycleTimes(F("Transaction Time"))
	{
		debug_i("Testing %sware SPI", SPISOFT_ENABLE ? "Soft" : "Hard");
	}

	void execute() override
	{
		System.setCpuFrequency(CycleTimer::cpuFrequency());

		// spi.setup(SpiBus::SPI3);
		REQUIRE(spi.begin());

		debug_w("Connected SCK %u, MISO %u, MOSI %u", spi.pins.sck, spi.pins.miso, spi.pins.mosi);

		settings.speed = 150e3;
		spi.beginTransaction(settings);
		settings.speed = 2e6;
		spi.beginTransaction(settings);
		settings.speed = 1e6;
		spi.beginTransaction(settings);
		settings.speed = 800e3;
		spi.beginTransaction(settings);
		settings.speed = 10000;
		spi.beginTransaction(settings);
		settings.speed = 30000;
		spi.beginTransaction(settings);
		settings.speed = 50000;
		spi.beginTransaction(settings);
		settings.speed = 1;
		spi.beginTransaction(settings);

#if SPISOFT_ENABLE && SPISOFT_CALIBRATE
		testSoftwareDelays();
#else
		testBitPatterns();
#endif
	}

#if SPISOFT_ENABLE && SPISOFT_CALIBRATE
	void testSoftwareDelays()
	{
		TEST_CASE("Software SPI delay")
		{
			debug_w("Connect scope and measure");
			settings.speed = 0;
			constexpr unsigned duration{5};
			constexpr unsigned loopInterval{250};
			loopCount = 0;
			delay = -1;
			timer.initializeMs<loopInterval>([this]() {
				if(loopCount-- == 0) {
					Serial.println();

					if(delay >= 100) {
						timer.stop();
						testBitPatterns();
						return;
					}

					if(delay >= 20) {
						delay += 10;
					} else if(delay < 12) {
						++delay;
					} else {
						delay = 20;
					}
					Serial.print("delay = ");
					Serial.print(delay);
					spi.setDelay(delay);
					loopCount = duration * 1000 / loopInterval;
				}

				spi.beginTransaction(settings);
				uint8_t data[]{0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};
				spi.transfer(data, sizeof(data));
				spi.endTransaction();

				m_putc('.');
			});
			timer.start();
			return pending();
		}
	}
#endif

	void testBitPatterns()
	{
		TEST_CASE("Bit Patterns")
		{
			debug_w("Connect scope and observe bit pattern");
			clearStats();
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
		settings.dataMode = SPI_MODE0;
		for(uint8_t bitOrder : {MSBFIRST, LSBFIRST}) {
			settings.bitOrder = bitOrder;
			beginTrans();
			spi.transfer32(0x00AA00AA);
			spi.transfer32(0x12345678);
			uint8_t data[]{0x12, 0x34, 0x56, 0x78, 0x9A};
			spi.transfer(data, ARRAY_SIZE(data));
			endTrans(13 * 8);
			delayMicroseconds(100);
		}

		delayMicroseconds(200);
		for(auto dataMode : {SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3}) {
			settings.bitOrder = MSBFIRST;
			settings.dataMode = dataMode;
			beginTrans();
			spi.transfer32(0xAA, 13);
			endTrans(13);
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
		m_puts("\r\n");
		printStats();

		loopbackTests();

		System.setCpuFrequency(CpuCycleClockNormal::cpuFrequency());
		spi.end();
		complete();
	}

	void loopbackTests()
	{
		if(!spi.loopback(true)) {
			debug_w("WARNING: SPI loopback not supported. Manual connection required.");
			debug_w("Connect MISO (GPIO%u) <-> MOSI (GPIO%u)", spi.pins.miso, spi.pins.mosi);
			allowFailure = true;
		}

		settings.speed = 8000000;
		settings.dataMode = SPI_MODE0;

		TEST_CASE("32-bit values")
		{
			clearStats();

			/*
			 * Note: Single-bit transfers fail on esp32c3, and RP2040 doesn't
			 * support less than 4 bits. So start at 4.
			 */
			for(auto bitOrder : {MSBFIRST, LSBFIRST}) {
				settings.bitOrder = bitOrder;
				for(auto bits : {4, 7, 8, 9, 15, 16, 17, 19, 23, 24, 25, 29, 30, 31}) {
					send(0, bits);
					send(0xffffffff, bits);
					send(0xaaaaaaaa, bits);
					send(0x55555555, bits);
					send(0x12345678, bits);
					send(~0x12345678, bits);
				}
			}

			printStats();
		}

		TEST_CASE("Byte sequences")
		{
			clearStats();

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

			printStats();
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

		beginTrans();
		spi.beginTransaction(settings);
		uint32_t inValue = spi.transfer32(outValue, bits);
		endTrans(bits);

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

		beginTrans();
		spi.transfer(bufptr, length);
		endTrans(length * 8);

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
	void clearStats()
	{
		cycleTimes.clear();
		totalBitCount = 0;
	}

	__forceinline void beginTrans()
	{
		cycleTimes.start();
		spi.beginTransaction(settings);
	}

	__forceinline void endTrans(size_t bitCount)
	{
		spi.endTransaction();
		cycleTimes.update();
		totalBitCount += bitCount;
	}

	void printStats()
	{
		Serial.println(cycleTimes);
		unsigned kbps = 1e6 * totalBitCount / cycleTimes.getTotalTime();
		Serial.print("Average bitrate: ");
		Serial.print(kbps);
		Serial.println(" kbit/s");
	}

	Timer timer;
	MinMaxTimes<CycleTimer> cycleTimes;
	size_t totalBitCount{0};
	SPISettings settings;
	unsigned loopCount{0};
	int delay{0};
	bool allowFailure{false};
};

void REGISTER_TEST(SPI)
{
#ifdef ARCH_HOST
	debug_i("Host has nothing useful to do here...");
#else
	registerGroup<SpiTest>();
#endif
}
