#include <gdb/gdb_hooks.h>

void gdb_enable(bool state)
{
}

GdbState gdb_present(void)
{
	return eGDB_NotPresent;
}

void __attribute__((weak)) gdb_on_attach(bool attached)
{
}

void gdb_detach(void)
{
}



unsigned __gdb_no_op(void)
{
	return 0;
}

//#define NOOP __attribute__((weak, alias("__gdb_no_op")))
//
//void gdb_on_attach(bool attached) NOOP;
