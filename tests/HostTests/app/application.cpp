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
#include <malloc_count.h>

Vector<TestGroupFactory> groupFactories;
static SimpleTimer taskTimer;
static unsigned taskIndex;
static NanoTime::Time<uint32_t> totalTestTime(NanoTime::Milliseconds, 0);

#define XX(t) extern void REGISTER_TEST(t);
TEST_MAP(XX)
#undef XX

static void registerTests()
{
#define XX(t) REGISTER_TEST(t);
	TEST_MAP(XX)
#undef XX
}

static void runNextGroup()
{
	if(taskIndex >= groupFactories.count()) {
		m_printf("\r\n\nTESTS COMPLETE\r\n\n");
		m_printf("Heap allocations: %u, total: %u bytes, peak: %u, current: %u\r\n", MallocCount::getAllocCount(),
				 MallocCount::getTotal(), MallocCount::getPeak(), MallocCount::getCurrent());
		m_printf("Total test time: %s\r\n\n", totalTestTime.value().toString().c_str());
		totalTestTime.time = 0;
#if RESTART_DELAY == 0
		System.restart();
#else
		taskIndex = 0;
		taskTimer.setIntervalMs<RESTART_DELAY>();
		taskTimer.startOnce();
#endif
	} else {
		auto factory = groupFactories[taskIndex++];
		factory()->commenceTest();
	}
}

void TestGroup::commenceTest()
{
	m_printf(_F("\r\n\r\n** Test Group: %s (%u of %u)**\r\n\r\n"), name.c_str(), taskIndex, groupFactories.count());
	groupTimer.start();
	state = State::running;
	execute();
	if(state != State::pending) {
		complete();
	}
}

void TestGroup::startItem(const String& tag)
{
	m_printf(_F("\r\n>> %s\r\n"), tag.c_str());
}

void TestGroup::complete()
{
	auto elapsed = groupTimer.elapsedTime();
	totalTestTime += elapsed;
	if(state == State::failed) {
		m_printf(_F("\r\n!!!! Test Group '%s' FAILED !!!!\r\n\r\n"), name.c_str());
	} else {
		m_printf(_F("\r\n** Test Group '%s' OK ** Elapsed: %s\r\n"), name.c_str(), elapsed.toString().c_str());
	}
	delete this;
	taskTimer.setIntervalMs<TEST_GROUP_INTERVAL>();
	taskTimer.startOnce();
}

void TestGroup::fail(const char* func)
{
	state = State::failed;
	m_printf(_F("FAIL in `%s`\r\n"), func);
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

	taskTimer.setCallback(runNextGroup);
	System.onReady(runNextGroup);
}
