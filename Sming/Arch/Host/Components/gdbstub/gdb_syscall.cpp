#include <gdb/gdb_syscall.h>
#include <cerrno>

int gdb_syscall(const GdbSyscallInfo&)
{
	errno = ENODEV;
	return -1;
}
