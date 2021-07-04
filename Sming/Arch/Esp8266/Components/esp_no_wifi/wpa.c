#include <stddef.h>
#include <stdint.h>

uint32_t os_random();

int os_get_random(unsigned char* buf, size_t len)
{
	uint32_t rnd = 0;
	for(size_t i = 0; i < len; ++i) {
		if(rnd == 0) {
			rnd = os_random();
		}
		*buf++ = rnd & 0xff;
		rnd >>= 8;
	}

	return len;
}