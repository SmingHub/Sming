#include <SmingCore.h>
#include <AnalogueReader.h>
#include <HardwareTimer.h>
#include <Services/Profiling/CpuUsage.h>

#ifdef ARCH_HOST
#include <Digital.h>
#include <AppDigiHooks.h>
#endif

namespace
{
/*
 * Hardware timer
 */
HardwareTimer1<TIMER_CLKDIV_16, eHWT_Maskable> hwtimer;

constexpr unsigned hwTimerInterval{250};		   ///< Hardware timer interval in microseconds
constexpr unsigned hwTimerReportInterval{1000000}; //< How often to print hardware timer count values

volatile unsigned hwTimerCount;

/*
 * Analogue reader task
 */
AnalogueReader<> reader;

/*
 * CPU usage monitor
 */
Profiling::CpuUsage cpuUsage;

/*
 * Demonstration of a simple delegate callback for the hardware timer.
 * Note that a callback function is faster, but as we're doing printing, etc.
 * this has to be executed in task context.
 */
void hwTimerDelegate(uint32_t count)
{
	static unsigned lastCount;
	auto diff = count - lastCount;

	static ElapseTimer timer;
	auto elapsed = timer.elapsedTime();

	Serial << _F("count = ") << count << _F(", hwTimerCount = ") << hwTimerCount << _F(", av. interval = ")
		   << (diff ? (elapsed / diff) : 0) << _F(", maxTasks = ") << System.getMaxTaskCount() << _F(", CPU usage = ")
		   << cpuUsage.getUtilisation() / 100.0 << '%' << endl;

	cpuUsage.reset();

	lastCount = count;
	timer.start();
}

void IRAM_ATTR hwTimerCallback()
{
	++hwTimerCount;

	static ElapseTimer timer(hwTimerReportInterval);
	if(timer.expired()) {
		unsigned count = hwTimerCount;
		//	System.queueCallback([count]() { hwTimerDelegate(count); });
		System.queueCallback(hwTimerDelegate, count);
		timer.start();
	}
}

void onReady()
{
	Serial.print("Calibrating CPU usage...");
	cpuUsage.begin([]() {

#ifndef DISABLE_WIFI
	// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifdef WIFI_SSID
		WifiStation.config(WIFI_SSID, WIFI_PWD);
#endif
		WifiStation.enable(true);
		WifiAccessPoint.enable(false);
		WifiEvents.onStationGotIP([](IpAddress ip, IpAddress netmask, IpAddress gateway) {
			Serial << _F("GOTIP - IP: ") << ip << _F(", mask: ") << netmask << _F(", gateway: ") << gateway << endl;
		});

#endif

		hwtimer.initializeUs<hwTimerInterval>(hwTimerCallback).start();
		reader.resume();
	});
}

} // namespace

void init()
{
	Serial.setTxBufferSize(1024);
	Serial.setTxWait(false); // Make sure debug output doesn't stall
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	// System.setCpuFrequency(CpuCycleClockFast::cpuFrequency());

	Serial.print(_F("Available heap: "));
	Serial.println(system_get_free_heap_size());

#ifdef ARCH_HOST
	setDigitalHooks(&appDigiHooks);
#endif

#ifndef DISABLE_WIFI
	WifiAccessPoint.enable(false);
	WifiStation.enable(false);
#endif

	System.onReady(onReady);
}
