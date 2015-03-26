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
    System.cpuUpdate();
    procTimer.initializeUs(5, blink).start();
    //Serial.println(System.getCpuFreq());
    Serial.println("ESP 8266 started.");
    Serial.print("CPU freq: ");
    Serial.println(System.getCpuFreq());
    pinMode(LED_PIN, OUTPUT);
    Serial.print("CPU freq: ");
    System.cpuUpdate();
    Serial.println(System.getCpuFreq());
    procTimer.initializeUs(1, blink).start();
}
