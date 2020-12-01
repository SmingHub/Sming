#include <gdb/gdb_syscall.h>

int gdb_syscall(const GdbSyscallInfo& info)
{
	errno = ENODEV;
	return -1;
}
