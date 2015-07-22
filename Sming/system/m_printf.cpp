/*
Modified: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 20.07.2015
Descr: embedded version of printf with float support
Contribution: simplify the original implementation even more e.g. remove some format sub-specifiers like
				-uppercase
				-left-justify
				-width and precision specified separately with * and .*
				-zeropadding
				-plus and space for sign display
*/

/* File : barebones/ee_printf.c
	This file contains an implementation of ee_printf that only requires a method to output a char to a UART without pulling in library code.

This code is based on a file that contains the following:
 Copyright (C) 2002 Michael Ringgaard. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.  
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.  
 3. Neither the name of the project nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission. 
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.

*/

#include <stdarg.h>
#include "../../SmingCore/SmingCore.h"


void (*cbc_printchar)(char ch) = uart_tx_one_char;

#define SIGN    	(1<<1)	/* Unsigned/signed long */
#define HEX_PREP 	(1<<5)	/* 0x */

typedef union {
long L;
float F;
} LF_t;



static const char *lower_digits = "0123456789abcdefghijklmnopqrstuvwxyz";

static size_t strnlen(const char *s, size_t count);

static size_t strnlen(const char *s, size_t count)
{
  const char *sc;
  for (sc = s; *sc != '\0' && count--; ++sc);
  return sc - s;
}


static char *ee_number(char *str, long num, int base, int type)
{
  char sign = ' ', tmp[66];
  int i = -1;

  if (base < 2 || base > 36) return 0;
  
  if (type & SIGN)
  {
    if (num < 0)
    {
      sign = '-';
      num = -num;
    }
  }

  if (num == 0)
    tmp[i++] = '0';
  else
  {
    while (num != 0)
    {
      tmp[i++] = lower_digits[((unsigned long) num) % (unsigned) base];
      num = ((unsigned long) num) / (unsigned) base;
    }
  }

  if (sign) *str++ = sign;
  
	if (base == 8)
	  *str++ = '0';
	else if (base == 16)
	{
	  *str++ = '0';
	  *str++ = 'x';
	}

  while (i-- > 0) *str++ = tmp[i];

  return str;
}




char* ftoa(char *buf, float f)
{
	long mantissa, int_part, frac_part;
	short exp2;
	LF_t x;
	uint8_t m, max = 6;

	do
	{
		if (f == 0.0)
		{
			*buf++ = '0';
			break;
		}
		else if(isnan(f))
		{
			*buf++ = 'n';
			*buf++ = 'a';
			*buf++ = 'n';
			break;
		}

		x.F = f;

		exp2 = (unsigned char) (x.L >> 23) - 127;
		mantissa = (x.L & 0xFFFFFF) | 0x800000;
		frac_part = 0;
		int_part = 0;

		if (exp2 >= 31)
		{
			*buf++ = 'I';
			*buf++ = 'n';
			*buf++ = 'f';
			break;
		}
		else if (exp2 < -23)
		{
			*buf++ = '-';
			*buf++ = 'I';
			*buf++ = 'n';
			*buf++ = 'f';
			break;
		}
		else if (exp2 >= 23)
			int_part = mantissa << (exp2 - 23);
		else if (exp2 >= 0)
		{
			int_part = mantissa >> (23 - exp2);
			frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
		}
		else
			/* if (exp2 < 0) */
			frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);



		if (x.L < 0)
			*buf++ = '-';

		if (int_part == 0)
			*buf++ = '0';
		else
		{
			buf = ee_number(buf, int_part, 10, 0);
		}
		*buf++ = '.';

		if (frac_part == 0)
			*buf++ = '0';
		else
		{
			/* print BCD */
			for (m = 0; m < max; m++)
			{
				/* frac_part *= 10; */
				frac_part = (frac_part << 3) + (frac_part << 1);

				*buf++ = (frac_part >> 24) + '0';
				frac_part &= 0xFFFFFF;
			}
			/* delete ending zeroes */
			for (--buf; buf[0] == '0' && buf[-1] != '.'; --buf)
				;
			++buf;
		}
	} while(0);

	return buf;

}




static int ee_vsprintf(char *buf, const char *fmt, va_list args)
{
  int len;
  unsigned long num;
  int i, base;
  char *str;
  const char *s;

  int flags;            // Flags to number()

  int field_width;      // Width of output field
  int precision;        // Min. # of digits for integers; max number of chars for from string
  int qualifier;        // 'h', 'l', or 'L' for integer fields

  for (str = buf; *fmt; fmt++)
  {
    if (*fmt != '%')
    {
      *str++ = *fmt;
      continue;
    }
                  
    // Process flags
    flags = 0;

    fmt++; // This also skips first '%'

    // Get field width
    field_width = -1;

    // Get the precision
    precision = 6;


    // Get the conversion qualifier
    qualifier = -1;
    if (*fmt == 'l' || *fmt == 'L')
    {
      qualifier = *fmt;
      fmt++;
    }

    // Default base
    base = 10;

    switch (*fmt)
    {
      case 'c':
        while (--field_width > 0) *str++ = ' ';
        *str++ = (unsigned char) va_arg(args, int);

        continue;

      case 's':
        s = va_arg(args, char *);
        if (!s) s = "<NULL>";
        len = strnlen(s, precision);
        while (len < field_width--) *str++ = ' ';
        for (i = 0; i < len; ++i) *str++ = *s++;

        continue;

      case 'p':

        str = ee_number(str, (unsigned long) va_arg(args, void *), 16, flags);
        continue;

      // Integer number formats - set up the flags and "break"
      case 'o':
        base = 8;
        break;

      case 'X':
      case 'x':
        base = 16;
        break;

      case 'd':
      case 'i':
        flags |= SIGN;
      case 'u':
        break;


      case 'f':
      case 'g':
      case 'G':
      case 'e':
      case 'E':
        str = ftoa(str, va_arg(args, double));
        continue;


      default:
        if (*fmt != '%') *str++ = '%';
        if (*fmt)
          *str++ = *fmt;
        else
          --fmt;
        continue;
    }

    if (qualifier == 'l')
      num = va_arg(args, unsigned long);
    else if (flags & SIGN)
      num = va_arg(args, int);
    else
      num = va_arg(args, unsigned int);

    str = ee_number(str, num, base, flags);
  }

  *str = '\0';
  return str - buf;
}


int m_printf(const char *fmt, ...)
{
  char buf[15*80],*p;
  va_list args;
  int n=0;

  va_start(args, fmt);
  ee_vsprintf(buf, fmt, args);
  va_end(args);
  p=buf;
  while (*p) {
	  cbc_printchar(*p);
	n++;
	p++;
  }

  return n;
}
