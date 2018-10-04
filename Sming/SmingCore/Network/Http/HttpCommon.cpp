/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 * 	httpGetErrnoName() and httpGetStatusText() functions added
 *
 ****/

#include "HttpCommon.h"

// Define flash strings and lookup table for HTTP error names
#define XX(_n, _s) static DEFINE_FSTR(__hpename_##_n, "HPE_" #_n);
HTTP_ERRNO_MAP(XX)
#undef XX

static const FlashString* const __hpenames[] PROGMEM = {
#define XX(_n, _s) FSTR_PTR(__hpename_##_n),
	HTTP_ERRNO_MAP(XX)
#undef XX
};

String httpGetErrnoName(enum http_errno err)
{
	if(err > HPE_UNKNOWN)
		return F("HPE_#") + String(err);

	return *__hpenames[err];
}

// Define flash strings and lookup table for HTTP error descriptions
#define XX(_n, _s) static DEFINE_FSTR(__hpedesc_##_n, _s);
HTTP_ERRNO_MAP(XX)
#undef XX

static const FlashString* const __hpedescriptions[] PROGMEM = {
#define XX(_n, _s) FSTR_PTR(__hpedesc_##_n),
	HTTP_ERRNO_MAP(XX)
#undef XX
};

String httpGetErrnoDescription(enum http_errno err)
{
	if(err > HPE_UNKNOWN)
		return F("HPE_#") + String(err);

	return *__hpedescriptions[err];
}

// Define flash strings for HTTP status codes
#define XX(_num, _name, _string) static DEFINE_FSTR(__hpstext##_num, #_string);
HTTP_STATUS_MAP(XX)
#undef XX

String httpGetStatusText(enum http_status code)
{
	switch(code) {
#define XX(_num, _name, _string)                                                                                       \
	case _num:                                                                                                         \
		return __hpstext##_num;
		HTTP_STATUS_MAP(XX)
#undef XX
	default:
		return F("<unknown_") + String(code) + '>';
	}
}
