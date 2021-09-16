/*
 * Provide overrides to direct any console writes e.g. via newlib printf. 
 */

#include <m_printf.h>
#include <unistd.h>
#include <errno.h>

ssize_t __wrap__write_r(struct _reent* r, int fd, const void* data, size_t size)
{
	(void)r;
	(void)fd; // Ignore, direct everything
	return m_nputs(data, size);
}

ssize_t __wrap__read_r(struct _reent* r, int fd, void* dst, size_t size)
{
	(void)r;
	(void)fd;
	(void)dst;
	(void)size;
	errno = ENOSYS;
	return -1;
}
