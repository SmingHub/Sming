/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * romfunc_md5.h - declarations for MD5 checksum calculation functions in ESP8266 ROM
 *
 ****/

#pragma once

#ifndef MD5_SIZE
#define MD5_SIZE 16
#elif MD5_SIZE != 16
#error Incompatible definition of MD5_SIZE
#endif

// see esptool/flasher_stub/rom_functions.h

typedef struct {
	uint32_t buf[4];
	uint32_t bits[2];
	uint8_t in[64];
} MD5Context;

void MD5Init(MD5Context* ctx);
void MD5Update(MD5Context* ctx, void* buf, uint32_t len);
void MD5Final(uint8_t digest[MD5_SIZE], MD5Context* ctx);
