#include <HostTests.h>
#include <esp_spi_flash.h>

namespace
{
String modeToString(SPIFlashMode mode)
{
	switch(mode) {
	case MODE_QIO:
		return _F("QIO");
	case MODE_QOUT:
		return _F("QOUT");
	case MODE_DIO:
		return _F("DIO");
	case MODE_DOUT:
		return _F("DOUT");
	case MODE_FAST_READ:
		return _F("FAST_READ");
	case MODE_SLOW_READ:
		return _F("SLOW_READ");
	default:
		return nullptr;
	}
}

String speedToString(SPIFlashSpeed speed)
{
	switch(speed) {
	case SPEED_40MHZ:
		return _F("40 MHz");
	case SPEED_26MHZ:
		return _F("26 MHz");
	case SPEED_20MHZ:
		return _F("20 MHz");
	case SPEED_80MHZ:
		return _F("80 MHz");
	default:
		return nullptr;
	}
}

String sizeToString(SPIFlashSize size)
{
	switch(size) {
	case SIZE_1MBIT:
		return _F("1 MBit");
	case SIZE_2MBIT:
		return _F("2 MBit");
	case SIZE_4MBIT:
		return _F("4 MBit");
	case SIZE_8MBIT:
		return _F("8 MBit");
	case SIZE_16MBIT:
		return _F("16 MBit");
	case SIZE_32MBIT:
		return _F("32 MBit");
	default:
		return nullptr;
	}
}

} // namespace

class SpiFlashTest : public TestGroup
{
public:
	SpiFlashTest() : TestGroup(_F("SPI Flash"))
	{
	}

	void execute() override
	{
		TEST_CASE("Query flash info")
		{
			auto flash_id = spi_flash_get_id();
			REQUIRE_NEQ(flash_id, 0);
			debug_i("flash_id = 0x%08x", flash_id);

			auto flash_size = flashmem_get_size_bytes();
			REQUIRE_NEQ(flash_size, 0);
			debug_i("flash_size = 0x%08x bytes, %u MBytes", flash_size, flash_size / 0x100000);

			auto info = flashmem_get_info();
			auto modeStr = modeToString(SPIFlashMode(info.mode));
			auto speedStr = speedToString(SPIFlashSpeed(info.speed));
			auto sizeStr = sizeToString(SPIFlashSize(info.size));
			PSTR_ARRAY(unk, "Unknown");
			Serial.print(F("Mode:  "));
			Serial.println(modeStr ?: unk);
			Serial.print(F("Speed: "));
			Serial.println(speedStr ?: unk);
			Serial.print(F("Size:  "));
			Serial.println(sizeStr ?: unk);
			REQUIRE(modeStr != nullptr && speedStr != nullptr && sizeStr != nullptr);
		}
	}
};

void REGISTER_TEST(SpiFlash)
{
	registerGroup<SpiFlashTest>();
}
