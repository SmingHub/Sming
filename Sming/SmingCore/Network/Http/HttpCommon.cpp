/*
 * HttpCommon.cpp
 *
 *  Created on: 26 Jul 2018
 *      Author: mikee47
 */

#include "HttpCommon.h"

// Define flash strings for each error value
#define XX(_n, _s) static const char __hpestr_##_n[] PROGMEM = "HPE_" #_n;
HTTP_ERRNO_MAP(XX)
#undef XX

// Lookup table in flash
static const char* const __hpe_table[] PROGMEM = {
#define XX(_n, _s) __hpestr_##_n,
	HTTP_ERRNO_MAP(XX)
#undef XX
};

/*
 * Return a string name of the given error.
 *
 * This replaces the one in http_parser module which uses a load of RAM.
 * We use their macro definitions though :-)
 *
 */
String httpGetErrnoName(enum http_errno err)
{
	if (err > HPE_UNKNOWN)
		return nullptr;

	char buffer[64];
	strncpy_P(buffer, __hpe_table[err], sizeof(buffer));
	return buffer;
}

String httpGetStatusText(enum http_status code)
{
	switch (code) {
#define XX(num, name, string)                                                                                          \
	case num:                                                                                                          \
		return F(#string);
		HTTP_STATUS_MAP(XX)
#undef XX
	default:
		return F("<unknown>");
	}
}
