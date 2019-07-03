/*
 * Evaluates relative speeds of various types of callback
 */

#include <SmingCore.h>
#include <Services/Profiling/ElapseTimer.h>
#include "callbacks.h"

#ifdef ARCH_HOST
const unsigned ITERATIONS = 10000000;
#else
const unsigned ITERATIONS = 100000;
#endif

typedef Delegate<void(int)> TestDelegate;
typedef void (*TestCallback)(int);

void evaluateSpeed()
{
	Serial.println();
	Serial.println();
	Serial.printf("Timings are in ns per loop, averaged over %u iterations\r\n", ITERATIONS);

	auto printTime = [](const char* name, unsigned elapsed) {
		unsigned ns = ((1000 * elapsed) + (ITERATIONS / 2)) / ITERATIONS;
		Serial.printf("%s: %u\r\n", name, ns);
	};

	const int testParam = 123;

	{
		TestCallback callback = callbackTest;
		ElapseTimer timer;
		for(unsigned i = 0; i < ITERATIONS; ++i) {
			callback(testParam);
		}
		auto elapsed = timer.elapsed();
		printTime("Callback", elapsed);
	}

	{
		TestDelegate callback = callbackTest;
		ElapseTimer timer;
		for(unsigned i = 0; i < ITERATIONS; ++i) {
			callback(testParam);
		}
		auto elapsed = timer.elapsed();
		printTime("Delegate (function)", elapsed);
	}

	{
		TestClass cls;
		auto callback = TestDelegate(&TestClass::callbackTest, &cls);
		ElapseTimer timer;
		for(unsigned i = 0; i < ITERATIONS; ++i) {
			callback(testParam);
		}
		auto elapsed = timer.elapsed();
		printTime("Delegate (method)", elapsed);
	}

	{
		TestClass cls;
		TestDelegate callback = std::bind(&TestClass::callbackTest, &cls, _1);
		ElapseTimer timer;
		for(unsigned i = 0; i < ITERATIONS; ++i) {
			callback(testParam);
		}
		auto elapsed = timer.elapsed();
		printTime("Delegate (bind)", elapsed);
	}
}
