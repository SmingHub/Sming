/*
 * Provide overrides to direct any console writes e.g. via newlib printf. 
 */

#include <m_printf.h>
#include <unistd.h>
#include <errno.h>

#define BUFMAX 16384
#define WRAP(x) __wrap_##x

ssize_t WRAP(_write_r)(struct _reent* r, int fd, const void* data, size_t size)
{
	(void)r;
	(void)fd; // Ignore, direct everything
	return m_nputs(data, size);
}

ssize_t WRAP(_read_r)(struct _reent* r, int fd, void* dst, size_t size)
{
	(void)r;
	(void)fd;
	(void)dst;
	(void)size;
	errno = ENOSYS;
	return -1;
}

size_t WRAP(putchar)(char c)
{
	return m_putc(c);
}

size_t WRAP(puts)(const char* str)
{
	return m_puts(str);
}

int WRAP(vprintf)(const char* format, va_list arg)
{
	return m_vprintf(format, arg);
}

int WRAP(printf)(char const* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int n = m_vprintf(fmt, args);
	va_end(args);
	return n;
}

int WRAP(vsnprintf)(char* buf, size_t maxLen, const char* fmt, va_list args)
{
	return m_vsnprintf(buf, maxLen, fmt, args);
}

int WRAP(snprintf)(char* buf, int length, const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	int n = m_vsnprintf(buf, length, fmt, args);
	va_end(args);

	return n;
}

int WRAP(vsprintf)(char* buf, const char* fmt, va_list args)
{
	return m_vsnprintf(buf, BUFMAX, fmt, args);
}

int WRAP(sprintf)(char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int n = m_vsnprintf(buf, BUFMAX, fmt, args);
	va_end(args);
	return n;
}
