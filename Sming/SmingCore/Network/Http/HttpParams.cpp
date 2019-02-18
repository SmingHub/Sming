/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpParams.cpp
 *
 * @author: 2018 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "HttpParams.h"
#include "../Services/WebHelpers/escape.h"
#include "Print.h"

size_t HttpParams::printTo(Print& p) const
{
	size_t charsPrinted = 0;
	for(unsigned i = 0; i < count(); i++) {
		if(i > 0)
			charsPrinted += p.print('&');
		charsPrinted += p.print(uri_escape(keyAt(i)));
		charsPrinted += p.print('=');
		charsPrinted += p.print(uri_escape(valueAt(i)));
	}

	return charsPrinted;
}
