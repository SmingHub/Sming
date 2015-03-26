#include <user_config.h>
#include <SmingCore/SmingCore.h>

#define LED_PIN 4 // GPIO2

Timer procTimer;
bool state = true;

void blink()
{
    digitalWrite(LED_PIN, state);
    state = !state;
}

void init()
{
	System.onReady(ets_wdt_disable);
    Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
    pinMode(LED_PIN, OUTPUT);
    System.cpuUpdate(160);
    procTimer.initializeUs(5, blink).start();
    //Serial.println(System.getCpuFreq());
}
