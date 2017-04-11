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

static void defaultPrintChar(uart_t *uart, char c) {
	return uart_tx_one_char(c);
}

void (*cbc_printchar)(uart_t *, char) = defaultPrintChar;
uart_t *cbc_printchar_uart = NULL;

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i = 0;
	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

void setMPrintfPrinterCbc(void (*callback)(uart_t *, char), uart_t *uart)
{
	cbc_printchar = callback;
	cbc_printchar_uart = uart;
}

void m_putc(char c)
{
	if (cbc_printchar)
		cbc_printchar(cbc_printchar_uart, c);
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
		cbc_printchar(cbc_printchar_uart, *p);
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

int m_vsnprintf(char *buf, size_t maxLen, const char *fmt, va_list args) {
	size_t size = 0;
	auto add = [&](char c) {
		if (++size < maxLen) *buf++ = c;
	};

	while (*fmt) {
		//	copy verbatim text
		if (*fmt != '%')  {
			add(*fmt++);
			continue;
		}
		fmt++;

		const char*	s;							// source for string copy
		char	tempNum[40];					// buffer for number conversion

		//	reset attributes to defaults
		bool	minus		= 0;
		uint8_t	base		= 10;
		int8_t	precision	= -1;
		int8_t	width		= 0;
		char	pad			= ' ';

		//	process flags
	DO_FLAGS:
		switch (*fmt) {
			case '-':
				minus = 1;

			case '+':
			case ' ':
			case '#':
				fmt++;
				goto DO_FLAGS;
		}

		//	process padding
		if (*fmt == '0') {
			pad = '0';
			fmt++;
		}

		//	process width ('*' is not supported yet)
		if ( is_digit(*fmt) ) {
			width = skip_atoi(&fmt);
		}

		//	process precision
		if( *fmt == '.' ) {
			fmt++;
			if ( is_digit(*fmt) ) precision = skip_atoi(&fmt);
		}

		//	ignore length
		while (*fmt == 'l' || *fmt == 'h' || *fmt == 'L') fmt++;

		//	process type
		switch (char f = *fmt++) {
			case '%':
				add('%');
				continue;

			case 'c':
				add( (unsigned char) va_arg(args, int) );
				continue;

			case 's': {
				s = va_arg(args, char *);

				if (!s) s = "(null)";
				size_t len = strlen(s);
				if (len > precision) len = precision;

				int padding = width - len;
				while (!minus && padding-- > 0) add(' ');
				while (len--)					add(*s++);
				while (minus && padding-- > 0)	add(' ');
				continue;
			}

			case 'p':
				s = ultoa((unsigned long) va_arg(args, void *), tempNum, 16);
				goto COPY;

			case 'd':
			case 'i':
				s = ltoa_wp(va_arg(args, int), tempNum, base, width, pad);
				goto COPY;

			case 'f':
				s = dtostrf_p(va_arg(args, double), width, precision, tempNum, pad);
				goto COPY;

			case 'o':
				base = 8;
				goto UNSIGNED;

			case 'x':
			case 'X':
				base = 16;
				goto UNSIGNED;

			case 'u':
			UNSIGNED:
				s = ultoa_wp(va_arg(args, unsigned int), tempNum, base, width, pad);

			COPY:
				while (*s) add(*s++);
				break;

			default:
				add('%');
				add(f);
		}
	}
	*buf = 0;
	return size;
}
