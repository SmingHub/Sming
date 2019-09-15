/*
 * Tests optimised time conversion code use for hardware timer.
 * We do this by evaluating various time values and comparing against floating-point
 * calculation.
 */

#include "common.h"
#include <HardwareTimer.h>
#include <Platform/Timers.h>
#include <malloc_count.h>

using Timer1TestApi = Timer1Api<TIMER_CLKDIV_16, eHWT_Maskable>;

class HardwareTimerTest : public CallbackTimer<Timer1TestApi>
{
public:
	// So we can keep track of how often timer's been called
	unsigned count = 0;
};

class CallbackTimerTest : public TestGroup
{
public:
	CallbackTimerTest() : TestGroup(_F("Callback Timers"))
	{
	}

	static void IRAM_ATTR timer1Callback(void* arg)
	{
		System.queueCallback(
			[](uint32_t param) {
				debugf("timer1 expired");
				auto tmr = reinterpret_cast<HardwareTimerTest*>(param);
				if(++tmr->count == 5) {
					tmr->stop();
				}
			},
			uint32_t(arg));
	}

	void execute() override
	{
		MallocCount::setLogThreshold(0);
		//		MallocCount::enableLogging(true);

		checkCallbackTimer<HardwareTimerTest>();
		checkCallbackTimer<SimpleTimer>();
		checkCallbackTimer<Timer>();

#define SHOW_SIZE(Type)                                                                                                \
	{                                                                                                                  \
		Serial.print("sizeof(" #Type ") = ");                                                                          \
		Serial.println(sizeof(Type));                                                                                  \
	}

		SHOW_SIZE(os_timer_t);
		SHOW_SIZE(OsTimerApi);
		SHOW_SIZE(SimpleTimer);
		SHOW_SIZE(OsTimer64Api<Timer>);
		SHOW_SIZE(Timer);
		SHOW_SIZE(AutoDeleteTimer);
		SHOW_SIZE(Timer1TestApi);
		SHOW_SIZE(HardwareTimerTest);

		timer1.initializeMs<500>(timer1Callback, &timer1);
		timer1.start();

		statusTimer.initializeMs<1000>([this]() {
			bool done = false;
			++statusTimerCount;
			//			if(timer32_count == 6) {
			if(activeTimerCount == 0) {
				Serial.print("statusTimer expired: ");
				Serial.println(statusTimer);
				statusTimer.stop();
				Serial.print("statusTimer stopped: ");
				Serial.println(statusTimer);

				// Release any allocated delegate memory
				timer64.setCallback(TimerDelegate(nullptr));
				done = true;
			}

			auto mem = MallocCount::getCurrent();
			String s = statusTimer;
			s += " fired, timercount = ";
			s += activeTimerCount;
			s += ", mem = ";
			s += mem;
			Serial.println(s);
			Serial.println(longTimer);

			if(done) {
				MallocCount::enableLogging(false);
				if(mem != memStart) {
					Serial.print("mem != memStart, memStart = ");
					Serial.println(memStart);
					TEST_ASSERT(false);
				}
				complete();
			}
		});
		statusTimer.start();

		Serial.println(statusTimer);

		{
			auto tmp = new Timer;
			tmp->initializeMs<1200>(
				[](void* arg) {
					auto self = static_cast<CallbackTimerTest*>(arg);
					debugf("%s fired", String(self->timer64).c_str());
				},
				this);
			tmp->startOnce();
		}

		if(1) {
			longTimer.setCallback([this]() {
				--activeTimerCount;
				auto ticks = Timer::Clock::ticks();
				Serial.println(longTimer.toString());
				Serial.print("Elapsed ticks = ");
				Serial.println(ticks - longStartTicks);
				debugf("Finally done!");
			});
			Serial.print("longTimer.maxTicks = ");
			Serial.println(longTimer.maxTicks());
			longTimer.setIntervalMs<15000>();
			longTimer.startOnce();
			longStartTicks = Timer::Clock::ticks();
			Serial.print("longTimer.start = ");
			Serial.println(longStartTicks);
			++activeTimerCount;
			Serial.println(longTimer.toString());
		}

		// Note memory usage before allocating timers
		memStart = MallocCount::getCurrent();

		AutoDeleteTimer::checkIntervalMs<100>();
		AutoDeleteTimer::checkIntervalMs<50000>();
		for(unsigned i = 0; i < 50; ++i) {
			auto timer = new AutoDeleteTimer;
			auto intervalMs = 100 * (i + 1);
			timer->initializeMs(intervalMs, [this]() { --activeTimerCount; }).startOnce();
			TEST_ASSERT(timer->getIntervalMs() == intervalMs);
			Serial.println(*timer);
			++activeTimerCount;
		}

		auto mem = MallocCount::getCurrent();
		Serial.print("Timers allocated, memStart = ");
		Serial.print(memStart);
		Serial.print(", now mem = ");
		Serial.print(mem);
		Serial.print(", used = ");
		Serial.println(mem - memStart);

		pending();
	}

	template <typename TimerType> static void checkCallbackTimer()
	{
		TimerType timer;

		Serial.print(timer);
		Serial.print(", maxTicks = ");
		Serial.print(TimerType::maxTicks());
		Serial.print(", maxTime = ");
		Serial.print(TimerType::Micros::MaxClockTime::value());
		Serial.println();

		//	CpuCycleTimer timer;
		//	Serial.print(timer);
		//	Serial.print(", maxClockTime = ");
		//	Serial.print(NanoTime::TimeValue(timer.unit(), timer.maxClockTime()));
		//	Serial.print(", ticksPerUnit = ");
		//	Serial.print(timer.ticksPerUnit());
		//	Serial.print(", maxInterval = ");
		//	Serial.print(timer.maxInterval());
		//	Serial.print(", margin = ");
		//	Serial.print(timer.margin().ticks());
		//	Serial.print(" ticks");
		//	Serial.println();

		//
		const auto time = NanoTime::time(NanoTime::Microseconds, 5000); //500020107;
		auto ticks = timer.usToTicks(time);
		Serial.print("time = ");
		Serial.print(time.toString());
		Serial.print(", ticks = ");
		Serial.print(ticks);
		Serial.print(", ");
		Serial.print(TimerType::Micros::ticksToTime(ticks).toString());
		Serial.println();

		//
		auto t1 = timer.micros().template timeConst<5000>();
		t1.check();
		Serial.print("t1 = ");
		Serial.print(t1.toString());
		Serial.print(", ");
		Serial.print(t1.clock().toString());
		Serial.print(", ticksPerUnit = ");
		Serial.print(t1.ticksPerUnit());
		Serial.print(", ticks = ");
		Serial.print(t1.ticks());
		Serial.print(", ");
		Serial.print(t1.clockTime());
		Serial.print(", ");
		Serial.print(t1.clockValue());
		Serial.println();

		//	ElapseTimer et;
		//	timer.reset<500000000>();
		//	while(!timer.expired()) {
		//	}
		//	auto elapsed = et.elapsedTime();
		//	Serial.print("Elapsed = ");
		//	Serial.println(NanoTime::time(et.unit(), elapsed));

		//	Ratio<uint32_t> ratio(NanoTime::Seconds);
		//	Serial.println(ratio);

		//	auto nanos = typename TimerType::template TicksConst<TimerType::maxTicks() + 1>::as<NanoTime::Nanoseconds>();
		//		using Nanos = NanoTime::TimeSource<typename TimerType::Clock, NanoTime::Nanoseconds, uint64_t>;

		typename TimerType::Clock::template TicksConst<timer.maxTicks() + 1> nanos;

		//		auto nanos = NanoTime::TicksConst<typename TimerType::Clock, timer.maxTicks() + 1>::as<NanoTime::NanoSeconds>();
		//		auto nanos = Nanos::template ticksConst<timer.maxTicks() + 1>();
		Serial.print("nanos = ");
		Serial.print(nanos.template as<NanoTime::Nanoseconds>().toString());
		Serial.println();

		Serial.print("interval = ");
		Serial.print(timer.getIntervalUs());
		Serial.print("us, ticks = ");
		Serial.print(timer.getInterval());
		Serial.println();
	}

private:
	Timer statusTimer;
	unsigned statusTimerCount = 0;
	Timer timer64;
	HardwareTimerTest timer1;
	Timer longTimer;
	uint32_t longStartTicks = 0;
	unsigned activeTimerCount = 0;
	uint32_t memStart = 0;
};

template <typename TimerApi> class CallbackTimerApiTest : public TestGroup
{
public:
	static constexpr unsigned iterations = 50;
	TimerApi api;
	CycleTimes times1, times2, times3, times4;

	CallbackTimerApiTest()
		: TestGroup(F("Callback timer API: ") + api.typeName()), times1("setCallback"), times2("setInterval"),
		  times3("arm"), times4("disarm")
	{
	}

	void execute()
	{
		api.setInterval(api.maxTicks());

		Serial.println(api);

		for(auto i = 0; i < iterations; ++i) {
			profile_setCallback();
			profile_setInterval();
			profile_arm();
			profile_disarm();
		}

		Serial.println(times1);
		Serial.println(times2);
		Serial.println(times3);
		Serial.println(times4);
		Serial.println();
	}

	void __attribute__((noinline)) profile_setCallback()
	{
		auto func = [](void*) {};
		times1.start();
		api.setCallback(func, nullptr);
		times1.update();
	}

	void __attribute__((noinline)) profile_setInterval()
	{
		times2.start();
		api.setInterval(api.maxTicks());
		times2.update();
	}

	void __attribute__((noinline)) profile_arm()
	{
		times3.start();
		api.arm(false);
		times3.update();
	}

	void __attribute__((noinline)) profile_disarm()
	{
		times4.start();
		api.disarm();
		times4.update();
	}
};

template <typename TimerType> class CallbackTimerSpeedTest : public TestGroup
{
public:
	static constexpr unsigned iterations = 50;
	TimerType timer;
	CycleTimes times1, times2a, times2b, times3a, times3b, times4, times5;

	CallbackTimerSpeedTest()
		: TestGroup(F("Callback timer speed: ") + TimerType::typeName()), times1("setCallback"),
		  times2a("setIntervalUs"), times2b("setIntervalUs<>"), times3a("setInterval"), times3b("setInterval<>"),
		  times4("start"), times5("stop")
	{
	}

	void execute()
	{
		Serial.println(timer);

		for(auto i = 0; i < iterations; ++i) {
			profile_setCallback();
			profile_setIntervalUs();
			profile_setIntervalUsT();
			profile_setInterval();
			profile_setIntervalT();
			profile_start();
			profile_stop();
		}
		Serial.println(times1);
		Serial.println(times2a);
		Serial.println(times2b);
		Serial.println(times3a);
		Serial.println(times3b);
		Serial.println(times4);
		Serial.println(times5);

		Serial.print("Combined set/start: ");
		Serial.println(times2a.getAverage() + times4.getAverage());
		Serial.print("Combined set/start, ticks: ");
		Serial.println(times3a.getAverage() + times4.getAverage());
		Serial.print("Combined set/start, templated: ");
		Serial.println(times2b.getAverage() + times4.getAverage());

		Serial.println();
	}

	void __attribute__((noinline)) profile_setCallback()
	{
		auto func = [](void*) {};
		times1.start();
		timer.setCallback(func);
		times1.update();
	}

	void __attribute__((noinline)) profile_setIntervalUs()
	{
		times2a.start();
		timer.setIntervalUs(1000000);
		times2a.update();
	}

	void __attribute__((noinline)) profile_setIntervalUsT()
	{
		times2b.start();
		timer.template setIntervalUs<1000000>();
		times2b.update();
	}

	void __attribute__((noinline)) profile_setInterval()
	{
		times3a.start();
		timer.setInterval(5000000);
		times3a.update();
	}

	void __attribute__((noinline)) profile_setIntervalT()
	{
		times3b.start();
		timer.template setInterval<5000000>();
		times3b.update();
	}

	void __attribute__((noinline)) profile_start()
	{
		times4.start();
		timer.start();
		times4.update();
	}

	void __attribute__((noinline)) profile_stop()
	{
		times5.start();
		timer.stop();
		times5.update();
	}
};

void REGISTER_TEST(timers)
{
	registerGroup<CallbackTimerApiTest<Timer1TestApi>>();
	registerGroup<CallbackTimerApiTest<OsTimerApi>>();
	registerGroup<CallbackTimerApiTest<OsTimer64Api<Timer>>>();

	registerGroup<CallbackTimerSpeedTest<HardwareTimerTest>>();
	registerGroup<CallbackTimerSpeedTest<SimpleTimer>>();
	registerGroup<CallbackTimerSpeedTest<Timer>>();

	registerGroup<CallbackTimerTest>();
}
