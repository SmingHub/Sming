#include <Libraries/WS2812/WS2812.h>

#define LED_PIN 2 // GPIO2

void init()
{
	while(true) {
		char buffer1[] = "\x40\x00\x00\x00\x40\x00\x00\x00\x40";
		ws2812_writergb(LED_PIN, buffer1, sizeof(buffer1));
		os_delay_us(500000);

		//We need to feed WDT.
		WDT.alive();

		char buffer2[] = "\x00\x40\x40\x40\x00\x40\x40\x40\x00";
		ws2812_writergb(LED_PIN, buffer2, sizeof(buffer2));
		os_delay_us(500000);
	}
}
