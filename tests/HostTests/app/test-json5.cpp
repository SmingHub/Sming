#include "common.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include <JsonObjectStream5.h>

void test_json5()
{
	testGroup("ArduinoJson5");

	// LONGLONG support
	startTest("LONGLONG support");
	{
		StaticJsonBuffer<256> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		const uint64_t testnum = 0x12345678ABCDEF99ULL;
		root["longtest"] = testnum;
		auto num = root.get<int64_t>("longtest");
		debug_i("int64 test: %s, 0x%08x%08x", num == testnum ? "OK" : "FAIL", uint32_t(num >> 32), uint32_t(num));
		assert(num == testnum);
	}
}
