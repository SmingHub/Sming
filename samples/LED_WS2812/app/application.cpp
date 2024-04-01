#include <Libraries/WS2812/WS2812.h>

#define LED_PIN 2 // GPIO2

namespace
{
SimpleTimer procTimer;
bool state;

void update()
{
	if(state) {
		char buffer2[] = "\x00\x40\x40\x40\x00\x40\x40\x40\x00";
		ws2812_writergb(LED_PIN, buffer2, sizeof(buffer2));
	} else {
		char buffer1[] = "\x40\x00\x00\x00\x40\x00\x00\x00\x40";
		ws2812_writergb(LED_PIN, buffer1, sizeof(buffer1));
	}
	state = !state;
}

} // namespace

void init()
{
	procTimer.initializeMs<500>(update).start();
}
