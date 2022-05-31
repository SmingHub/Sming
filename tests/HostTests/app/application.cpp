/*
 * Test framework
 *
 * See SmingTest library for details
 *
 */

#include <SmingTest.h>
#include <modules.h>

#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID"
#define WIFI_PWD "PleaseEnterPass"
#endif

#define XX(t) extern void REGISTER_TEST(t);
TEST_MAP(XX)
#undef XX

namespace
{
void registerTests()
{
#define XX(t)                                                                                                          \
	REGISTER_TEST(t);                                                                                                  \
	debug_i("Test '" #t "' registered");
	TEST_MAP(XX)
#undef XX
}

void testsComplete()
{
#if RESTART_DELAY == 0
	System.restart();
#else
	SmingTest::runner.execute(testsComplete, RESTART_DELAY);
#endif
}

void beginTests()
{
	SmingTest::runner.setGroupIntervalMs(TEST_GROUP_INTERVAL);
	System.onReady([]() { SmingTest::runner.execute(testsComplete); });
}

} // namespace

void init()
{
	Serial.setTxBufferSize(1024);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	debug_e("WELCOME to SMING! Host Tests application running.");

	spiffs_mount();
	fileSystemFormat();

	registerTests();

#ifdef DISABLE_WIFI
	beginTests();
#else
	WifiAccessPoint.enable(false);
	if(netif_default == nullptr) {
		WifiStation.enable(false);
		beginTests();
	} else {
		WifiStation.enable(true);
		WifiStation.config(WIFI_SSID, WIFI_PWD);
		WifiEvents.onStationGotIP([](IpAddress ip, IpAddress netmask, IpAddress gateway) { beginTests(); });
	}
#endif
}
