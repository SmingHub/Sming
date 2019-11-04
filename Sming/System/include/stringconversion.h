/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StringConversion.h
 *
 *  Created on: 28 ���. 2015 �.
 *      Author: Anakonda
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Since C does not support default func parameters, we define inline versions which
 * translate to ltoa_wp, ultoa_wp, lltoa_wp and ulltoa_wp functions.
 */

extern char* ltoa_wp(long val, char* buffer, int base, int width, char pad);

static inline char* ltoa_w(long val, char* buffer, int base, int width)
{
	return ltoa_wp(val, buffer, base, width, ' ');
}

static inline char* ltoa (long val, char* buffer, int base)
{
	return ltoa_wp(val, buffer, base, 0, ' ');
}

static inline char* itoa (int val, char* buffer, int base)
{
	return ltoa_wp(val, buffer, base, 0, ' ');
}

extern char* ultoa_wp(unsigned long val, char* buffer, unsigned int base, int width, char pad);

static inline char* ultoa_w(unsigned long val, char* buffer, unsigned int base, int width)
{
	return ultoa_wp(val, buffer, base, width, ' ');
}

static inline char* ultoa(unsigned long val, char* buffer, unsigned int base)
{
	return ultoa_wp(val, buffer, base, 0, ' ');
}

extern char* lltoa_wp(long long val, char* buffer, int base, int width, char pad);

static inline char* lltoa_w(long long val, char* buffer, int base, int width)
{
	return lltoa_wp(val, buffer, base, width, ' ');
}

static inline char* lltoa(long long val, char* buffer, int base)
{
	return lltoa_wp(val, buffer, base, 0, ' ');
}


extern char* ulltoa_wp(unsigned long long val, char* buffer, unsigned int base, int width, char pad);

static inline char* ulltoa_w(unsigned long long val, char* buffer, unsigned int base, int width)
{
	return ulltoa_wp(val, buffer, base, width, ' ');
}

static inline char* ulltoa(unsigned long long val, char* buffer, unsigned int base)
{
	return ulltoa_wp(val, buffer, base, 0, ' ');
}


extern char* itoa(int, char*, int);

extern char *dtostrf_p(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer, char pad);

static inline char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer)
{
	return dtostrf_p(floatVar, minStringWidthIncDecimalPoint, numDigitsAfterDecimal, outputBuffer, ' ');
}

long atol(const char *nptr);
extern long os_strtol(const char* str, char** endptr, int base);
extern double os_strtod(const char* str, char** endptr);
double os_atof(const char* s);

#ifdef __cplusplus
}
#endif
