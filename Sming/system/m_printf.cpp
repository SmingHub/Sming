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

static void defaultPrintChar(uart_t *uart, char c) {
	return uart_tx_one_char(c);
}

void (*cbc_printchar)(uart_t *, char) = defaultPrintChar;
uart_t *cbc_printchar_uart = NULL;

#define SIGN    	(1<<1)	/* Unsigned/signed long */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define MIN(a, b)   ( (a) < (b) ? (a) : (b) )

static int skip_atoi(const char **s)
{
	int num = 0;
	while (1) {
		char c = pgm_read_byte(*s);
		if ( !is_digit(c) ) return num;
		num = num * 10 + (c - '0');
		++*s;
	}
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

int m_vsnprintf(char *buf, size_t maxLen, const char *fmt, va_list args)
{
    size_t size = 0;
    auto add = [&](char c) {
        if (++size < maxLen) *buf++ = c;
    };

    while ( char f = pgm_read_byte(fmt) ) {
        //  copy verbatim text
        if (f != '%')  {
            add(f);
			fmt++;
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

        while (char f = pgm_read_byte(fmt)) {
            if (f == '-')           minus = 1;
            else if (f == '+')      ;           // ignored
            else if (f == ' ')      ;           // ignored
            else if (f == '#')      ;           // ignored
            else                    break;
            fmt++;
        }

        //  process padding
        if (pgm_read_byte(fmt) == '0') {
            pad = '0';
            fmt++;
        }

        //  process width ('*' is not supported yet)
        if ( is_digit(pgm_read_byte(fmt)) ) {
            width = skip_atoi(&fmt);
        }

        //  process precision
        if( pgm_read_byte(fmt) == '.' ) {
            fmt++;
            if ( is_digit(pgm_read_byte(fmt)) ) precision = skip_atoi(&fmt);
        }

        //  ignore length specifier
        for ( char f = pgm_read_byte(fmt); f=='l' || f=='h' || f=='L'; fmt++) ;

        //  process type
        switch (char f = pgm_read_byte(fmt++)) {
            case '%':
                add('%');
                continue;

            case 'c':
                add( (unsigned char) va_arg(args, int) );
                continue;

            case 's':
            case 'S': {
                s = va_arg(args, char *);

                if (!s) s = PSTR("(null)");
                size_t len = strlen_P(s);
                if (len > precision) len = precision;

                int padding = width - len;
                while (!minus && padding-- > 0) add(' ');
                while (len--)                   add(pgm_read_byte(s++));
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
