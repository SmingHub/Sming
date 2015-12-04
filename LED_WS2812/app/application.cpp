#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <WS2812/WS2812.h>

#define LED_PIN 2 // GPIO2

Timer procTimer;
int step;

void writeWS2812()
{
    if ((step++) & 1) {
        char buffer1[] = "\x40\x00\x00\x00\x40\x00\x00\x00\x40";
        ws2812_writergb(LED_PIN, buffer1, sizeof(buffer1));
    }
    else
    {
        char buffer2[] = "\x00\x40\x40\x40\x00\x40\x40\x40\x00";
        ws2812_writergb(LED_PIN, buffer2, sizeof(buffer2));
    }        
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	procTimer.initializeMs(500, writeWS2812).start();   // every 0.5 seconds
}
