#include <SmingCore.h>
#include <Services/Profiling/TaskStat.h>

namespace
{
Profiling::TaskStat taskStat(Serial);
SimpleTimer statTimer;

void handleNotification(uint32_t param)
{
	debug_i("NOTIFY %u", param);
}

// Code to run on second CPU
void IRAM_ATTR app2_main(void*)
{
	static unsigned count;

	// Using a polled timer has virtually zero overhead as it accesses timing hardware directly
	PeriodicFastUs timer(100);

	for(;;) {
		if(!timer.expired()) {
			continue;
		}

		if(count % 10000 == 0) {
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

	// Provide some periodic status so we can see what tasks are active
	statTimer.initializeMs<2000>([]() { taskStat.update(); });
	statTimer.start();

	// Create task running on second core
	const unsigned stackSize = 2048;
	const unsigned priority = 5;
	xTaskCreatePinnedToCore(app2_main, "Sming2", stackSize, nullptr, priority, nullptr, 1);
}
