/*
 * StringConversion.h
 *
 *  Created on: 28 џэт. 2015 у.
 *      Author: Anakonda
 */

#ifndef INCLUDE_STRINGCONVERSION_H_
#define INCLUDE_STRINGCONVERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char *nptr); // Already implemented

extern char* ltoa_w (long, char*, int, int width);
extern char* ltoa (long, char*, int);

extern char* ultoa_w(unsigned long val, char* buffer, unsigned int base, int width);
extern char* ultoa(unsigned long val, char* buffer, unsigned int base);

#define itoa ltoa
extern char *dtostrf(double floatVar, int minStringWidthIncDecimalPoint, int numDigitsAfterDecimal, char *outputBuffer);
long atol(const char *nptr);
extern long os_strtol(const char* str, char** endptr, int base);
extern double os_strtod(const char* str, char** endptr);
double os_atof(const char* s);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_STRINGCONVERSION_H_ */
