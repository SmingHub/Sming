#include <SmingCore.h>
#include <Services/Profiling/TaskStat.h>

#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID"
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
Profiling::TaskStat taskStat(Serial);
SimpleTimer statTimer;

const uint8_t outputPin = 23;
bool nextOutputState;

const unsigned periodMicroseconds = 100;
const unsigned timerInterval = periodMicroseconds / 2;
const unsigned countsPerSecond = 1'000'000 / timerInterval;

void handleNotification(uint32_t param)
{
	debug_i("NOTIFY %u", param);
}

// Code to run on second CPU
void IRAM_ATTR app2_main(void*)
{
	static unsigned count;

	// We're going to toggle an IO pin very fast to evaluate jitter
	pinMode(outputPin, OUTPUT);

	// Using a polled timer has virtually zero overhead as it accesses timing hardware directly
	PeriodicFastUs timer(timerInterval);

	for(;;) {
		if(!timer.expired()) {
			continue;
		}

		// NB. Can use IDF GPIO calls or direct hardware access if required
		digitalWrite(outputPin, nextOutputState);
		nextOutputState = !nextOutputState;

		if(count % countsPerSecond == 0) {
			System.queueCallback(handleNotification, count);

			debug_i("TICK");

			// This is required if CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1=y
			// WDT.alive();
		}
		++count;
	}
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	Serial.println(_F("ESP32 dual-core demo."));

#ifndef DISABLE_WIFI
	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
#endif

	/*
		Provide some periodic status so we can see what tasks are active.
		Note that this causes the CPU to stall for about 30ms so by default leave it disabled.
	*/
	statTimer.initializeMs<2000>([]() { taskStat.update(); });
	// statTimer.start();

	/*
		Create task running on second core.
		As this is the only task allocated to the second CPU (idle task disabled)
		it will run without interference from the scheduler.
	 */
	const unsigned stackSize = 2048;
	const unsigned priority = 5;
	xTaskCreatePinnedToCore(app2_main, "Sming2", stackSize, nullptr, priority, nullptr, 1);
}
