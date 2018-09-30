/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/SmingHub/Sming
License: MIT
Date: 21.12.2015
Descr: embedded very simple version of printf with float support
*/
#ifndef _M_PRINTF_
#define _M_PRINTF_

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief callback type to output a string of data
 *  @param param
 *  @param str data to send
 *  @param length number of characters to write
 *  @retval number of characters written, which may be less than the requested size
 *  @note data does not need to be nul terminated and may contain any 8-bit values including nul
 */
typedef size_t (*nputs_callback_t)(void* param, const char* str, size_t length);

/** @brief set the character output routine
 *  @param callback
 *  @param param
 */
void m_setPuts(nputs_callback_t callback, void* param);

int m_vsnprintf(char *buf, size_t maxLen, const char *fmt, va_list args);
int m_snprintf(char* buf, int length, const char *fmt, ...);
int m_printf(char const*, ...);
int m_vprintf ( const char * format, va_list arg );

/** @brief output a single character
 *  @param c
 *  @retval size_t 1 on success, 0 if the character could not be output
 */
size_t m_putc(char c);

/** @brief output a text string
 *  @param str the text
 *  @param length length of text, excluding nul terminator
 *  @retval size_t number of characters actually output
 *  @note nul terminator is optional
 */
size_t m_nputs(const char* str, size_t length);

static inline size_t m_puts(const char* str)
{
	return m_nputs(str, strlen(str));
}

#ifdef __cplusplus
#define M_PRINTHEX_BYTESPERLINE = 16
#else
#define M_PRINTHEX_BYTESPERLINE
#endif

/** @brief output a block of data in hex format
 *  @param tag brief name to display with the data block
 *  @param data
 *  @param len
 *  @param bytesPerLine If non-zero, data will be output in block separated by carriage return
 *  @note intended for debugging
 */
void m_printHex(const char* tag, const void* data, size_t len, size_t bytesPerLine M_PRINTHEX_BYTESPERLINE);

#ifdef __cplusplus
}
#endif

#endif /*_M_PRINTF_*/
