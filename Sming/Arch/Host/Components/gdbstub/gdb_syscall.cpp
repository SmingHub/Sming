#include <gdb/gdb_syscall.h>
#include <errno.h>

int gdb_syscall(const GdbSyscallInfo&)
{
	errno = ENODEV;
	return -1;
}
