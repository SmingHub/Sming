// ---------------------------------------------------------------------------------------------------
// -- This WS2812 code must be compiled with -O2 to get the timing right.  Read this:
// -- http://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

#include "WS2812.h"

// The ICACHE_FLASH_ATTR is there to trick the compiler and get the very first pulse width correct.
static void ICACHE_FLASH_ATTR send_ws_0(uint8_t gpio)
{
    uint8_t i;
    i = 3; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
    i = 8; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

static void ICACHE_FLASH_ATTR send_ws_1(uint8_t gpio)
{
    uint8_t i;
    i = 7; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << gpio);
    i = 5; while (i--) GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << gpio);
}

// Byte triples in the buffer are interpreted as R G B values and sent to the hardware as G R B.
int ICACHE_FLASH_ATTR ws2812_writergb(uint8_t gpio, char *buffer, size_t length)
{
    // Initialize the output pin:
    pinMode(gpio, OUTPUT);
    digitalWrite(gpio, 0);

    // Ignore incomplete Byte triples at the end of buffer:
    length -= length % 3;

    // Rearrange R G B values to G R B order needed by WS2812 LEDs:
    size_t i;
    for (i = 0; i < length; i += 3) {
        const char r = buffer[i];
        const char g = buffer[i + 1];
        buffer[i] = g;
        buffer[i + 1] = r;
    }

    // Do not remove these:
    os_delay_us(1);
    os_delay_us(1);

    // Send the buffer:
    noInterrupts();
    const char * const end = buffer + length;
    while (buffer != end) {
        uint8_t mask = 0x80;
        while (mask) {
            (*buffer & mask) ? send_ws_1(gpio) : send_ws_0(gpio);
            mask >>= 1;
        }
        ++buffer;
    }
    interrupts();
}
