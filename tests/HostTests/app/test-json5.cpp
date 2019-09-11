#include "common.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include <JsonObjectStream5.h>

class Json5Test : public TestGroup
{
public:
	Json5Test() : TestGroup(_F("ArduinoJson5"))
	{
	}

	void execute() override
	{
		// LONGLONG support
		startTest("LONGLONG support");
		{
			StaticJsonBuffer<256> jsonBuffer;
			JsonObject& root = jsonBuffer.createObject();
			const uint64_t testnum = 0x12345678ABCDEF99ULL;
			root["longtest"] = testnum;
			auto num = root.get<int64_t>("longtest");
			debug_i("int64 test: %s, 0x%08x%08x", num == testnum ? "OK" : "FAIL", uint32_t(num >> 32), uint32_t(num));
			TEST_ASSERT(num == testnum);
		}
	}
};

void REGISTER_TEST(json5)
{
	registerGroup<Json5Test>();
}
