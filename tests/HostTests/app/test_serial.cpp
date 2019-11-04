#include "common.h"

#include <SerialBuffer.h>

class SerialTest : public TestGroup
{
public:
	SerialTest() : TestGroup(_F("Serial"))
	{
	}

	void execute() override
	{
		startTest("SerialBuffer write/read");

		static constexpr size_t BUFSIZE = 128;
		SerialBuffer txbuf;
		txbuf.resize(BUFSIZE);

		auto read = [&]() {
			String s;
			void* data;
			unsigned count;
			while((count = txbuf.getReadData(data)) != 0) {
				s.concat(static_cast<const char*>(data), count);
				txbuf.skipRead(count);
			}
			return s;
		};

		REQUIRE(txbuf.getFreeSpace() == BUFSIZE - 1);
		REQUIRE(txbuf.available() == 0);

		for(unsigned i = 0; i < 64; ++i) {
			TEST_ASSERT(txbuf.writeChar('A') == 1);
		}
		REQUIRE(txbuf.available() == 64);
		REQUIRE(txbuf.getFreeSpace() == BUFSIZE - 1 - 64);
		REQUIRE(read().length() == 64);

		String compareBuffer;
		String readBuffer;
		for(unsigned i = 0; i < 10; ++i) {
			m_printf("txfree = %u\n", txbuf.getFreeSpace());
			for(char c = 'a'; c <= 'z'; ++c) {
				if(txbuf.getFreeSpace() < 10) {
					readBuffer += read();
				}
				txbuf.writeChar(c);
				compareBuffer += c;
			}
			txbuf.writeChar('\n');
			compareBuffer += '\n';
		}
		readBuffer += read();
		REQUIRE(txbuf.available() == 0);
		REQUIRE(compareBuffer == readBuffer);
	}
};

void REGISTER_TEST(serial)
{
	registerGroup<SerialTest>();
}
