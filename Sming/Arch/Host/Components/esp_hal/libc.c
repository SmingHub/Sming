
#include "include/esp_libc.h"

#include <time.h>
#include <stdlib.h>

/* Random numbers */

uint32_t os_random(void)
{
	srand(time(NULL));
	return rand();
}

int os_get_random(uint8_t* buf, size_t len)
{
	srand(time(NULL));

	// RAND_MAX is 0x7fff
	uint16_t n = rand();
	for(size_t i = 0; i < len; ++i) {
		if(i % 2 == 0)
			n = rand();
		buf[i] = n;
		n >>= 8;
	}

	// Success
	return 0;
}

/* Misc */

void ets_install_putc1(void (*p)(char c))
{
	//
}

void system_set_os_print(bool onoff)
{
	//
}

