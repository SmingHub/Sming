#include "FakePgmSpace.h"

#ifdef ICACHE_FLASH

uint8_t pgm_read_byte(const void* pgm_addr) {
	uint8_t tmp[4];
	uint32_t addr = (uint32_t)pgm_addr;
	ets_memcpy(&tmp, (const void *)(addr & ~3), 4);
	return tmp[addr & 3];
}

uint16_t pgm_read_word(const void* pgm_addr) {
	uint8_t tmp[8];
	uint32_t addr = (uint32_t)pgm_addr;
	uint8_t offset = addr & 3;
	ets_memcpy(&tmp, (const void *)(addr & ~3), offset == 3 ? 8 : 4);
	return tmp[offset] | (tmp[offset + 1] << 8);
}

uint32_t pgm_read_dword(const void* pgm_addr) {
	uint8_t tmp[8];
	uint32_t addr = (uint32_t)pgm_addr;
	uint8_t offset = addr & 3;
	ets_memcpy(&tmp, (const void *)(addr & ~3), offset == 0 ? 4 : 8);
	return tmp[offset] | (tmp[offset + 1] << 8) | (tmp[offset + 2] << 16) | (tmp[offset + 3] << 24);
}

float pgm_read_float(const void* pgm_addr) {
	union {
		float f;
		uint32_t dw;
	} tmp;
	tmp.dw = pgm_read_dword(pgm_addr);
	return tmp.f;
}

#endif
