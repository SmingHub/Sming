/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * strerror.c - microc library doesn't contain strerror_r
 *
 ****/

#include <m_printf.h>

int __xpg_strerror_r(int err, char* buf, size_t bufSize)
{
	m_snprintf(buf, bufSize, "ERROR #%u", err);
	if(buf != NULL && bufSize > 0) {
		buf[bufSize - 1] = '\0';
	}
	return 0;
}

int strerror_r(int err, char* buf, size_t bufSize) __attribute__((weak, alias("__xpg_strerror_r")));
