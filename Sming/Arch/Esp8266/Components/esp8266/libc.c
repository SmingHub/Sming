/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * libc.c - amendments to C runtime code
 *
 */

int* __errno(void)
{
	static int errno_s = 0;
	return &errno_s;
}
