/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 20.07.2015
Descr: embedded very simple version of printf with float support
*/

#include <stdarg.h>
#include "osapi.h"

#define MPRINTF_BUF_SIZE 256

#define OVERFLOW_GUARD 24

void (*cbc_printchar)(char ch) = uart_tx_one_char;

#define SIGN    	(1<<1)	/* Unsigned/signed long */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define MIN(a, b)   ( (a) < (b) ? (a) : (b) )

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

void m_putc(char c)
{
	if (cbc_printchar)
		cbc_printchar(c);
}

/**
 * @fn int m_snprintf(char* buf, int length, const char *fmt, ...);
 *
 * @param buf - destination buffer
 * @param length - destination buffer size
 * @param fmt - printf compatible format string
 *
 * @retval int - number of characters written
 */
int m_snprintf(char* buf, int length, const char *fmt, ...)
{
	char *p;
	va_list args;
	int n = 0;

	va_start(args, fmt);
	n = m_vsnprintf(buf, length, fmt, args);
	va_end(args);

	return n;
}

int m_vprintf ( const char * format, va_list arg )
{
	if(!cbc_printchar)
	{
		return 0;
	}

	char buf[MPRINTF_BUF_SIZE], *p;

	int n = 0;
	m_vsnprintf(buf, sizeof(buf), format, arg);

	p = buf;
	while (p && n < sizeof(buf) && *p)
	{
		cbc_printchar(*p);
		n++;
		p++;
	}

	return n;
}

/**
 * @fn int m_printf(const char *fmt, ...);
 *
 * @param fmt - printf compatible format string
 *
 * @retval int - number of characters written to console
 */
int m_printf(const char* fmt, ...)
{
	int n=0;

	if(!fmt)
		return 0;

	va_list args;
	va_start(args, fmt);

	n = m_vprintf(fmt, args);

	va_end(args);

	return n;
}

int m_vsnprintf(char *buf, size_t maxLen, const char *fmt, va_list args)
{
	int i, base, flags;
	char *str;
	const char *s;
	int8_t precision, width;
	char pad;

	char tempNum[40];

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
		pad = ' ';
		base = 10;
        bool minus = 0;

		do
		{
            if ('-' == *fmt) minus = 1, fmt++;
            
			//skip width and flags data - not supported
			while ('+' == *fmt || '#' == *fmt || '*' == *fmt || 'l' == *fmt)
				fmt++;

			if (is_digit(*fmt)) {
				if (*fmt == '0') {
					pad = '0';
					fmt++;
				}
				width = skip_atoi(&fmt);
			}

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

		case 's': {
			s = va_arg(args, char *);

			if (!s) s = "(null)";
            size_t len = strlen(s);
            len     = MIN( len,   precision );
            len     = MIN( len,   maxLen - size_t(str - buf) - OVERFLOW_GUARD);
            width   = MIN( width, maxLen - size_t(str - buf) - OVERFLOW_GUARD);

            int padding = width - len;
            while (!minus && padding-- > 0) *str++ = ' ';
            while (len--) *str++ = *s++;
            while (minus && padding-- > 0) *str++ = ' ';

			continue;
        }    

		case 'p':
			s = ultoa((unsigned long) va_arg(args, void *), tempNum, 16);
			while (*s && (maxLen - (uint32_t)(str - buf) > OVERFLOW_GUARD))
				*str++ = *s++;
			continue;

		case 'o':
			base = 8;
			break;

		case 'x':
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		case 'f':

			s = dtostrf_p(va_arg(args, double), width, precision, tempNum, pad);
			while (*s && (maxLen - (uint32_t)(str - buf) > OVERFLOW_GUARD))
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
			s = ltoa_wp(va_arg(args, int), tempNum, base, width, pad);
		else
			s = ultoa_wp(va_arg(args, unsigned int), tempNum, base, width, pad);

		while (*s && (maxLen - (uint32_t)(str - buf) > OVERFLOW_GUARD))
			*str++ = *s++;
	}

	*str = '\0';
	return str - buf;
}
