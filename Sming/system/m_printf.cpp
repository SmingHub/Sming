/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 20.07.2015
Descr: embedded very simple version of printf with float support
*/

#include <stdarg.h>
#include "../../SmingCore/SmingCore.h"

#define MPRINTF_BUF_SIZE 256

#define OVERFLOW_GUARD 24

void (*cbc_printchar)(char ch) = uart_tx_one_char;

#define SIGN    	(1<<1)	/* Unsigned/signed long */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i = 0;
	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

void setMPrintfPrinterCbc(void (*callback)(char))
{
	cbc_printchar = callback;
}

int m_printf(const char *fmt, ...)
{
	if(!cbc_printchar)
	{
		return 0;
	}

	char buf[MPRINTF_BUF_SIZE], *p;
	va_list args;
	int n = 0;

	va_start(args, fmt);
	m_vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	p = buf;
	while (*p)
	{
		cbc_printchar(*p);
		n++;
		p++;
	}

	return n;
}

int m_vsnprintf(char *buf, size_t maxLen, const char *fmt, va_list args)
{
	int i, base, flags;
	char *str;
	const char *s;
	int8_t precision, width;

	char tempNum[24];

	for (str = buf; *fmt; fmt++)
	{
		if(maxLen - (uint32_t)(str - buf) < OVERFLOW_GUARD)
		{
			*str++ = '(';
			*str++ = '.';
			*str++ = '.';
			*str++ = '.';
			*str++ = ')';

			//mark end of string
			*str = '\0';

			//return maximum buffer len, so caller can detect not_enough_space
			return maxLen;
		}

		if (*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}

		flags = 0;
		fmt++; // This skips first '%'

		//reset attributes to defaults
		precision = -1;
		width = 0;
		base = 10;

		do
		{
			//skip width and flags data - not supported
			while ('+' == *fmt || '-' == *fmt || '#' == *fmt || '*' == *fmt || 'l' == *fmt)
				fmt++;

			if (is_digit(*fmt))
				width = skip_atoi(&fmt);

			if('.' == *fmt)
			{
				fmt++;
				if (is_digit(*fmt))
					precision = skip_atoi(&fmt);
			}
			else
				break;
		}while(1);

		switch (*fmt)
		{
		case 'c':
			*str++ = (unsigned char) va_arg(args, int);
			continue;

		case 's':
			s = va_arg(args, char *);

			if (!s)
			{
				s = "<NULL>";
			}
			else
			{
				while (*s)
					*str++ = *s++;
			}

			continue;

		case 'p':
			s = ultoa((unsigned long) va_arg(args, void *), tempNum, 16);
			while (*s)
				*str++ = *s++;
			continue;

		case 'o':
			base = 8;
			break;

		case 'x':
		case 'X':
			*str++ = '0';
			*str++ = 'x';
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		case 'f':

			s = dtostrf(va_arg(args, double), width, precision, tempNum);
			while (*s)
				*str++ = *s++;
			continue;

		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}

		if (flags & SIGN)
			s = ltoa_w(va_arg(args, int), tempNum, base, width);
		else
			s = ultoa_w(va_arg(args, unsigned int), tempNum, base, width);

		while (*s)
			*str++ = *s++;
	}

	*str = '\0';
	return str - buf;
}
