/*
 * Test framework
 *
 * See SmingTest library for details
 *
 */

#include <SmingTest.h>
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

#ifndef DISABLE_WIFI
	WifiStation.enable(false, false);
	WifiAccessPoint.enable(false, false);
#endif

	registerTests();

	SmingTest::runner.setGroupIntervalMs(TEST_GROUP_INTERVAL);
	System.onReady([]() { SmingTest::runner.execute(testsComplete); });
}
