/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 20.07.2015
Descr: embedded very simple version of printf with float support
*/

#include "m_printf.h"
#include "c_types.h"
#include "stringconversion.h"
#include "stringutil.h"

#define MPRINTF_BUF_SIZE 256

/*
 * Callback to output a single character.
 * Use m_setPutchar to specify.
 * See HardwareSerial for example.
 * By default, output is disabled here.
 * Startup behaviour is defined in user_init() in user_main.cpp.
 */
static struct
{
	putchar_callback_t callback;
	void* param;
} _putchar;

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_print(c) ((c) >= ' ' && (c) <= '~')

static int skip_atoi(const char** s)
{
	int i = 0;
	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

void m_setPutchar(putchar_callback_t callback, void* param)
{
	_putchar = {callback, param};
}

void m_putc(char c)
{
	if (_putchar.callback)
		_putchar.callback(_putchar.param, c);
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
int m_snprintf(char* buf, int length, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int n = m_vsnprintf(buf, length, fmt, args);
	va_end(args);
	return n;
}

int m_vprintf(const char* format, va_list arg)
{
	if (!_putchar.callback)
		return 0;

	char buf[MPRINTF_BUF_SIZE];
	m_vsnprintf(buf, sizeof(buf), format, arg);
	return m_puts(buf, sizeof(buf));
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
	if (!_putchar.callback || !fmt)
		return 0;

	va_list args;
	va_start(args, fmt);
	int n = m_vprintf(fmt, args);
	va_end(args);
	return n;
}

int m_vsnprintf(char* buf, size_t maxLen, const char* fmt, va_list args)
{
	size_t size = 0;
	auto add = [&](char c) {
		if (++size < maxLen)
			*buf++ = c;
	};

	while (*fmt) {
		//  copy verbatim text
		if (*fmt != '%') {
			add(*fmt++);
			continue;
		}
		fmt++;

		// source for string copy
		const char* s;
		// buffer for number conversion
		char tempNum[40];

		//  reset attributes to defaults
		bool minus = 0;
		uint8_t ubase = 0;
		int8_t precision = -1;
		int8_t width = 0;
		char pad = ' ';

		while (char f = *fmt) {
			if (f == '-')
				minus = 1;
			else if (f == '+')
				; // ignored
			else if (f == ' ')
				; // ignored
			else if (f == '#')
				; // ignored
			else
				break;
			fmt++;
		}

		//  process padding
		if (*fmt == '0') {
			pad = '0';
			fmt++;
		}

		//  process width ('*' is not supported yet)
		if (is_digit(*fmt))
			width = skip_atoi(&fmt);

		//  process precision
		if (*fmt == '.') {
			fmt++;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
		}

		//  ignore length
		while (*fmt == 'l' || *fmt == 'h' || *fmt == 'L')
			fmt++;

		//  process type
		switch (char f = *fmt++) {
		case '%':
			add('%');
			continue;

		case 'c':
			add((unsigned char)va_arg(args, int));
			continue;

		case 's': {
			s = va_arg(args, char*);
			if (!s)
				s = "(null)";
			size_t len = strlen(s);
			if (precision >= 0 && (int)len > precision)
				len = precision;

			int padding = width - len;
			while (!minus && padding-- > 0)
				add(' ');
			while (len--)
				add(*s++);
			while (minus && padding-- > 0)
				add(' ');
			continue;
		}

		case 'p':
			s = ultoa((unsigned long)va_arg(args, void*), tempNum, 16);
			break;

		case 'd':
		case 'i':
			s = ltoa_wp(va_arg(args, int), tempNum, 10, width, pad);
			break;

		case 'f':
			s = dtostrf_p(va_arg(args, double), width, precision, tempNum, pad);
			break;

		case 'o':
			ubase = 8;
			break;

		case 'x':
		case 'X':
			ubase = 16;
			break;

		case 'u':
			ubase = 10;
			break;

		default:
			add('%');
			add(f);
			continue;
		}

		//  format unsigned numbers
		if (ubase)
			s = ultoa_wp(va_arg(args, unsigned int), tempNum, ubase, width, pad);

		//  copy string to target
		while (*s)
			add(*s++);
	}
	*buf = 0;
	return size;
}

int m_puts(const char* str, int maxlen)
{
	if (!_putchar.callback || !str)
		return 0;

	const char* p = str;
	const char* pend = str + maxlen;
	while (p < pend && *p)
		m_putc(*p++);

	return p - str;
}

void m_printHex(const char* tag, const void* data, size_t len)
{
	auto buf = static_cast<const unsigned char*>(data);

	m_puts(tag, 32);
	m_putc(':');
	for (size_t i = 0; i < len; ++i) {
		m_putc(' ');
		m_putc(hexchar(buf[i] >> 4));
		m_putc(hexchar(buf[i] & 0x0f));
	}
	m_putc('\n');

	// Output ASCII on line below
	while (*tag++)
		m_putc(' ');
	m_putc(' ');
	for (size_t i = 0; i < len; ++i) {
		m_putc(' ');
		m_putc(' ');
		char c = buf[i];
		m_putc(is_print(c) ? c : ' ');
	}
	m_putc('\n');
}
