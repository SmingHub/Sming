#include <user_config.h>

#include <WS2812/WS2812.h>  //this includes SmingCore.h, which pulls in everything

#define LED_PIN 2 // GPIO2

Timer procTimer;
bool state = true;

void blink()
{
        char buffer1[] = "\x40\x00\x00\x00\x40\x00\x00\x00\x40";
        char buffer2[] = "\x00\x40\x40\x40\x00\x40\x40\x40\x00";
        if(state == 0){
        ws2812_writergb(LED_PIN, buffer1, sizeof(buffer1));
	} else {
        ws2812_writergb(LED_PIN, buffer2, sizeof(buffer2));
	}	
        state = !state;
}

void init()
{
        pinMode(LED_PIN, OUTPUT);
        procTimer.initializeMs(1000, blink).start();
}

