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

int atoi(const char *nptr); // Already implemented

extern char* ltoa_wp(long val, char* buffer, int base, int width, char pad);
extern char* ltoa_w (long, char*, int, int width);
extern char* ltoa (long, char*, int);

extern char* ultoa_wp(unsigned long val, char* buffer, unsigned int base, int width, char pad);
extern char* ultoa_w(unsigned long val, char* buffer, unsigned int base, int width);
extern char* ultoa(unsigned long val, char* buffer, unsigned int base);

extern char* itoa(int, char*, int);

extern char *dtostrf_p(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer, char pad);
extern char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer);
long atol(const char *nptr);
extern long os_strtol(const char* str, char** endptr, int base);
extern double os_strtod(const char* str, char** endptr);
double os_atof(const char* s);

#ifdef __cplusplus
}
#endif
