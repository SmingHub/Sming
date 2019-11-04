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
			REQUIRE(root.get<int64_t>("longtest") == testnum);
		}
	}
};

void REGISTER_TEST(json5)
{
	registerGroup<Json5Test>();
}
