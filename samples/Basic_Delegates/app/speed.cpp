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

static uint32_t __forceinline getCycleCount()
{
#ifdef ARCH_ESP8266
	uint32_t ccount;
	__asm__ __volatile__("rsr %0,ccount" : "=a"(ccount));
	return ccount;
#else
	return NOW();
#endif
}

static void printTime(const char* name, unsigned elapsed)
{
	Serial.printf("%s: %u\r\n", name, elapsed / ITERATIONS);
}

static void __attribute__((noinline)) evaluateCallback(const char* name, TestCallback callback, int testParam)
{
	unsigned startTicks = getCycleCount();
	for(unsigned i = 0; i < ITERATIONS; ++i) {
		callback(testParam);
	}
	unsigned elapsed = getCycleCount() - startTicks;
	printTime(name, elapsed);
}

static void __attribute__((noinline)) evaluateDelegate(const char* name, TestDelegate delegate, int testParam)
{
	unsigned startTicks = getCycleCount();
	for(unsigned i = 0; i < ITERATIONS; ++i) {
		delegate(testParam);
	}
	unsigned elapsed = getCycleCount() - startTicks;
	printTime(name, elapsed);
}

void evaluateSpeed()
{
	Serial.println();
	Serial.println();
	Serial.printf("Timings are in CPU cycles per loop, averaged over %u iterations\r\n", ITERATIONS);

	int testParam = 123;

	evaluateCallback("Callback", callbackTest, testParam);

	evaluateDelegate("Delegate (function)", callbackTest, testParam);

	auto lambda = [testParam](int) { callbackTest(testParam); };
	evaluateDelegate("Delegate (lambda)", lambda, 0);

	TestClass cls;
	evaluateDelegate("Delegate (method)", TestDelegate(&TestClass::callbackTest, &cls), testParam);

	evaluateDelegate("Delegate (bind)", std::bind(&TestClass::callbackTest, &cls, _1), testParam);
}
