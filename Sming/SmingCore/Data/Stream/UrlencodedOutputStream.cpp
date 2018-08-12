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

#include "../../../Services/WebHelpers/escape.h"

UrlencodedOutputStream::UrlencodedOutputStream(const HttpParams& params)
{
	int maxLength = 0;
	for (int i = 0; i < params.count(); i++) {
		int kLength = params.keyAt(i).length();
		int vLength = params.valueAt(i).length();
		if (maxLength < vLength || maxLength < kLength) {
			maxLength = (kLength < vLength ? vLength : kLength);
		}
	}

	char buffer[maxLength * 4 + 1];
	for (int i = 0, max = params.count(); i < max; i++) {
		String key = params.keyAt(i);
		String value = params.valueAt(i);

		char* temp = uri_escape(buffer, maxLength + 1, value.c_str(), value.length());
		String write = params.keyAt(i) + "=" + String(temp);
		if (i + 1 != max) {
			write += "&";
		}
		if (stream.write((uint8_t*)write.c_str(), write.length()) != write.length()) {
			break;
		}
	}
}
