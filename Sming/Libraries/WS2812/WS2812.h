#ifndef WS2812_h
#define WS2812_h

#include "../../SmingCore/SmingCore.h"

// Byte triples in the buffer are interpreted as R G B values and sent to the hardware as G R B.
int ICACHE_FLASH_ATTR ws2812_writergb(uint8_t gpio, char *buffer, size_t length);

#endif
