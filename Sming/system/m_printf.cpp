/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 20.07.2015
Descr: embedded very simple version of printf with float support
*/

#include "m_printf.h"
#include "stringconversion.h"
#include "stringutil.h"
#include <algorithm>

#define MPRINTF_BUF_SIZE 256

/*
 * Callback to output characters.
 * Use m_setPuts to specify.
 * See HardwareSerial for example.
 * By default, output is disabled here.
 * Startup behaviour is defined in user_init() in user_main.cpp.
 */
static struct
{
	nputs_callback_t callback;
	void* param;
} _puts;


#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_print(c) ((c) >= ' ' && (c) <= '~')

static int skip_atoi(const char **s)
{
	int i = 0;
	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

void m_setPuts(nputs_callback_t callback, void* param)
{
	_puts = { callback, param };
}

size_t m_putc(char c)
{
	return _puts.callback ? _puts.callback(_puts.param, &c, 1) : 0;
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
	va_list args;

	va_start(args, fmt);
	int n = m_vsnprintf(buf, length, fmt, args);
	va_end(args);

	return n;
}

int m_vprintf ( const char * format, va_list arg )
{
	if (!_puts.callback)
	{
		return 0;
	}

	char buf[MPRINTF_BUF_SIZE];
	int len = m_vsnprintf(buf, sizeof(buf), format, arg);
	return _puts.callback(_puts.param, buf, std::min((size_t)len, sizeof(buf)));
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
	if (!_puts.callback || !fmt)
		return 0;

	va_list args;
	va_start(args, fmt);
	int n = m_vprintf(fmt, args);
	va_end(args);
	return n;
}

int m_vsnprintf(char *buf, size_t maxLen, const char *fmt, va_list args)
{
    size_t size = 0;
    auto add = [&](char c) {
        if (++size < maxLen) *buf++ = c;
    };

    while (*fmt) {
        //  copy verbatim text
        if (*fmt != '%')  {
            add(*fmt++);
            continue;
        }
        fmt++;

        const char* s;                          // source for string copy
        char    tempNum[40];                    // buffer for number conversion

        //  reset attributes to defaults
        bool    minus       = 0;
        uint8_t ubase       = 0;
        int8_t  precision   = -1;
        int8_t  width       = 0;
        char    pad         = ' ';

        while (char f = *fmt) {
            if (f == '-')           minus = 1;
            else if (f == '+')      ;           // ignored
            else if (f == ' ')      ;           // ignored
            else if (f == '#')      ;           // ignored
            else                    break;
            fmt++;
        }

        //  process padding
        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }

        //  process width ('*' is not supported yet)
        if ( is_digit(*fmt) ) {
            width = skip_atoi(&fmt);
        }

        //  process precision
        if( *fmt == '.' ) {
            fmt++;
            if ( is_digit(*fmt) ) precision = skip_atoi(&fmt);
        }

        //  ignore length
        while (*fmt == 'l' || *fmt == 'h' || *fmt == 'L') fmt++;

        //  process type
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
                if (precision >= 0 && (int)len > precision) len = precision;

                int padding = width - len;
                while (!minus && padding-- > 0) add(' ');
                while (len--)                   add(*s++);
                while (minus && padding-- > 0)  add(' ');
                continue;
            }

            case 'p':
                s = ultoa((unsigned long) va_arg(args, void *), tempNum, 16);
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
        if (ubase) s = ultoa_wp(va_arg(args, unsigned int), tempNum, ubase, width, pad);

        //  copy string to target
        while (*s) add(*s++);
    }
    *buf = 0;
    return size;
}

size_t m_nputs(const char* str, size_t length)
{
	return _puts.callback ? _puts.callback(_puts.param, str, length) : 0;
}


void m_printHex(const char* tag, const void* data, size_t len, size_t bytesPerLine)
{
	auto buf = static_cast<const unsigned char*>(data);

	auto taglen = strlen(tag);
	size_t offset = 0;
	while (offset < len) {
		if (offset == 0) {
			m_nputs(tag, taglen);
			m_putc(':');
		}
		else {
			for (size_t  i = 0; i <= taglen; ++i)
				m_putc(' ');
		}

		size_t n = len - offset;
		if (bytesPerLine && n > bytesPerLine)
			n = bytesPerLine;

		for (size_t i = 0; i < n; ++i) {
			m_putc(' ');
			m_putc(hexchar(buf[offset + i] >> 4));
			m_putc(hexchar(buf[offset + i] & 0x0f));
		}
		m_putc('\n');

		// Output ASCII on line below
		for (size_t  i = 0; i <= taglen; ++i)
			m_putc(' ');
		for (size_t i = 0; i < n; ++i) {
			m_putc(' ');
			m_putc(' ');
			char c = buf[offset + i];
			m_putc(is_print(c) ? c : ' ');
		}
		m_putc('\n');

		offset += n;
	}
}
