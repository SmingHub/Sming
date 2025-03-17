#include <SmingCore.h>

void handleNotification(uint32_t param)
{
	Serial << "NOTIFY " << param << endl;
}

// Code to run on second CPU
void IRAM_ATTR app2_main(void*)
{
	static unsigned count;

	for(;;) {
		System.queueCallback(handleNotification, count);
		++count;
		delay(500);
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	Serial.println(_F("ESP32 dual-core demo."));

	const unsigned stackSize = 2048;
	const unsigned priority = 5;
	xTaskCreatePinnedToCore(app2_main, "Sming2", stackSize, nullptr, priority, nullptr, 1);
}
