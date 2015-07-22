/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 20.07.2015
Descr: embedded very simple version of printf with float support
*/

#include <stdarg.h>
#include "../../SmingCore/SmingCore.h"

void (*cbc_printchar)(char ch) = uart_tx_one_char;

#define SIGN    	(1<<1)	/* Unsigned/signed long */

static const char *lower_digits = "0123456789abcdefghijklmnopqrstuvwxyz";

static char *ee_number(char *str, long num, int base, int type)
{
  char sign = ' ', tmp[66];
  int i = 0;

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

	size_t n = 0;
	uint8_t digits = 6;

	do
	{

	  if (isnan(f))
	  {
		  *buf++ = 'n'; *buf++ = 'a'; *buf++ = 'n';
		  break;
	  }
	  if (isinf(f))
	  {
		  *buf++ = 'i'; *buf++ = 'n'; *buf++ = 'f';
		  break;
	  }
	  if (f > 4294967040.0)
	  {
		  *buf++ = 'o'; *buf++ = 'v'; *buf++ = 'f';
		  break;
	  }
	  if (f <-4294967040.0)
	  {
		  *buf++ = '-'; *buf++ = 'o'; *buf++ = 'v'; *buf++ = 'f';
		  break;
	  }

	  // Handle negative numbers
	  if (f < 0.0)
	  {
		  *buf++ = '-';
	    f = -f;
	  }

	  // Round correctly so that print(1.999, 2) prints as "2.00"
	  double rounding = 0.0000005;

	  f += rounding;

	  // Extract the integer part of the number and print it
	  unsigned long int_part = (unsigned long)f;
	  double remainder = f - (double)int_part;

	  buf = ee_number(buf, int_part, 10, 0);

	  // Print the decimal point, but only if there are digits beyond
	  *buf++ = '.';

	  // Extract digits from the remainder one at a time
	  while (digits-- > 0)
	  {
	    remainder *= 10.0;
	    uint8_t toPrint = uint8_t(remainder);
	    *buf++ = toPrint + '0';
	    remainder -= toPrint;
	  }

		/* delete ending zeroes */
		for (--buf; buf[0] == '0' && buf[-1] != '.'; --buf)
			;
		++buf;

	}
	while(0);

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

    fmt++; // This skips first '%'

    //actual percent sign
    if(*fmt == '%')
    {
    	*str++ = *fmt;
    	continue;
    }

    while((*fmt >= '0' && *fmt <= '9') || '.' == *fmt || '+' == *fmt || '-' == *fmt || '#' == *fmt || '*' == *fmt)
    	fmt++;

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
        *str++ = (unsigned char) va_arg(args, int);
        continue;

      case 's':
        s = va_arg(args, char *);

        if (!s)
        	s = "<NULL>";

        for (i = 0; *s; ++i) *str++ = *s++;

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
