#include "include/sys/pgmspace.h"

/*
 * TODO: If pointer mapped read-only then we can say it's flash.
 */
bool isFlashPtr(const void* ptr)
{
	return false;
}
