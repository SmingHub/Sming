/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 * 	httpGetErrorName(), httpGetErrorDescription() and httpGetStatusText() functions added
 *
 ****/

#include "HttpCommon.h"

// Define flash strings and lookup table for HTTP error names
#define XX(_n, _s) static DEFINE_FSTR(hpename_##_n, "HPE_" #_n);
HTTP_ERRNO_MAP(XX)
#undef XX

static FSTR_TABLE(hpeNames) = {
#define XX(_n, _s) FSTR_PTR(hpename_##_n),
	HTTP_ERRNO_MAP(XX)
#undef XX
};

String httpGetErrorName(enum http_errno err)
{
	if(err > HPE_UNKNOWN)
		return F("HPE_#") + String(err);

	return *hpeNames[err];
}

// Define flash strings and lookup table for HTTP error descriptions
#define XX(_n, _s) static DEFINE_FSTR(hpedesc_##_n, _s);
HTTP_ERRNO_MAP(XX)
#undef XX

static FSTR_TABLE(hpeDescriptions) = {
#define XX(_n, _s) FSTR_PTR(hpedesc_##_n),
	HTTP_ERRNO_MAP(XX)
#undef XX
};

String httpGetErrorDescription(enum http_errno err)
{
	if(err > HPE_UNKNOWN)
		return F("HPE_#") + String(err);

	return *hpeDescriptions[err];
}

// Define flash strings for HTTP status codes
#define XX(_num, _name, _string) static DEFINE_FSTR(hpsText_##_num, #_string);
HTTP_STATUS_MAP(XX)
#undef XX

String httpGetStatusText(enum http_status code)
{
	switch(code) {
#define XX(_num, _name, _string)                                                                                       \
	case _num:                                                                                                         \
		return hpsText_##_num;
		HTTP_STATUS_MAP(XX)
#undef XX
	default:
		return F("<unknown_") + String(code) + '>';
	}
}
