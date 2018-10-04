/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "UrlencodedOutputStream.h"
#include "../Services/WebHelpers/escape.h"

/*
 * @todo Revise this so stream produces encoded output line-by-line, rather than all at once.
 */

UrlencodedOutputStream::UrlencodedOutputStream(const HttpParams& params)
{
	for(unsigned i = 0; i < params.count(); i++) {
		if(i > 0)
			stream.write('&');

		stream.print(uri_escape(params.keyAt(i)));
		stream.print('=');
		stream.print(uri_escape(params.valueAt(i)));
	}
}
