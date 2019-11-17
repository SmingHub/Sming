/*
 * Test framework
 *
 * Related tests should be grouped into a single module.
 * The test code should be in a separate file called `test-XXX.cpp`
 * Each group must have a single entry function `void test_XXX()` - define using REGISTER_TEST macro
 * The name of the test must be added to the TEST_MAP definition below.
 *
 * See common.h for further details.
 */

#include "common.h"
#include "modules.h"

#define XX(t) extern void REGISTER_TEST(t);
TEST_MAP(XX)
#undef XX

static void registerTests()
{
#define XX(t) REGISTER_TEST(t);
	TEST_MAP(XX)
#undef XX
}

static void testsComplete()
{
#if RESTART_DELAY == 0
	System.restart();
#else
	SmingTest::runner.execute(testsComplete, RESTART_DELAY);
#endif
}

void init()
{
	Serial.setTxBufferSize(1024);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	spiffs_mount();

	WifiStation.enable(false, false);
	WifiAccessPoint.enable(false, false);

	registerTests();

	SmingTest::runner.setGroupIntervalMs(TEST_GROUP_INTERVAL);
	System.onReady([]() { SmingTest::runner.execute(testsComplete); });
}
