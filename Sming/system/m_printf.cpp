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

static int m_vsprintf(char *buf, const char *fmt, va_list args)
{
	int i, base, flags, qualifier;
	char *str;
	const char *s;

	char tempNum[24];

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
		if (*fmt == '%')
		{
			*str++ = *fmt;
			continue;
		}

		//skip precision, width and flags data - not supported
		while ((*fmt >= '0' && *fmt <= '9') || '.' == *fmt || '+' == *fmt
				|| '-' == *fmt || '#' == *fmt || '*' == *fmt)
			fmt++;

		// Get the conversion qualifier
		qualifier = -1;
		if (*fmt == 'l' || *fmt == 'L')
		{
			qualifier = 'l';
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

			s = dtostrf(va_arg(args, double), 0, 6, tempNum);
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

		if (qualifier == 'l')
			s = ultoa(va_arg(args, unsigned long), tempNum, base);
		else if (flags & SIGN)
			s = ltoa(va_arg(args, int), tempNum, base);
		else
			s = ultoa(va_arg(args, unsigned int), tempNum, base);

		while (*s)
			*str++ = *s++;
	}

	*str = '\0';
	return str - buf;
}


int m_printf(const char *fmt, ...)
{
	char buf[15 * 80], *p;
	va_list args;
	int n = 0;

	va_start(args, fmt);
	m_vsprintf(buf, fmt, args);
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
