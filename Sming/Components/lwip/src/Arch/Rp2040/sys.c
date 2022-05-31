#include <lwip/sys.h>

/* lwip has provision for using a mutex, when applicable */
sys_prot_t sys_arch_protect(void)
{
	return 0;
}

void sys_arch_unprotect(sys_prot_t pval)
{
	(void)pval;
}

/* lwip needs a millisecond time source */
uint32_t sys_now(void)
{
	extern uint32_t millis(void);
	return millis();
}
