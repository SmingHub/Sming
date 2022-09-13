/*
 * Evaluates relative speeds of various types of callback
 */

#include <SmingCore.h>
#include "callbacks.h"

#ifdef ARCH_HOST
const unsigned ITERATIONS = 10000000;
#else
const unsigned ITERATIONS = 100000;
#endif

using TestDelegate = Delegate<void(int)>;
using TestCallback = void (*)(int);

// Use for high resolution loop timing
static CpuCycleTimer timer;

static void printTime(const char* name, unsigned ticks)
{
	Serial << name << ": " << ticks << " cycles, " << timer.ticksToTime(ticks).toString() << endl;
}

static void __noinline evaluateCallback(const char* name, TestCallback callback, int testParam)
{
	timer.start();
	for(unsigned i = 0; i < ITERATIONS; ++i) {
		callback(testParam);
	}
	unsigned ticks = timer.elapsedTicks();
	printTime(name, ticks / ITERATIONS);
}

static void __noinline evaluateDelegate(const char* name, TestDelegate delegate, int testParam)
{
	timer.start();
	for(unsigned i = 0; i < ITERATIONS; ++i) {
		delegate(testParam);
	}
	unsigned ticks = timer.elapsedTicks();
	printTime(name, ticks / ITERATIONS);
}

void evaluateSpeed()
{
	Serial.println();
	Serial.println();
	Serial << _F("Timings are in CPU cycles per loop, averaged over ") << ITERATIONS << _F(" iterations") << endl;

	int testParam = 123;

	evaluateCallback("Callback", callbackTest, testParam);

	evaluateDelegate("Delegate (function)", callbackTest, testParam);

	auto lambda = [testParam](int) { callbackTest(testParam); };
	evaluateDelegate("Delegate (lambda)", lambda, 0);

	TestClass cls;
	evaluateDelegate("Delegate (method)", TestDelegate(&TestClass::callbackTest, &cls), testParam);

	evaluateDelegate("Delegate (bind)", std::bind(&TestClass::callbackTest, &cls, _1), testParam);
}
