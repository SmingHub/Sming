#include <user_config.h>
#include <SmingCore/SmingCore.h>
//#include <system/include/esp_systemapi.h>

#define LED_PIN 4 // GPIO4
#define BOARD_ESP01

Timer procTimer;
bool state = true;

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void init()
{
	char buf[64];
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	Serial.println("System started.");

	os_sprintf(buf,"CPU: %d Mhz",System.getCpuFrequency());
	Serial.println(buf);

	System.onReady(ets_wdt_disable);
	pinMode(LED_PIN, OUTPUT);
	System.setCpuFrequency(eCF_160MHz);

	os_sprintf(buf,"CPU: %d Mhz",System.getCpuFrequency());
	Serial.println(buf);
	procTimer.initializeUs(100, blink).start();
}
