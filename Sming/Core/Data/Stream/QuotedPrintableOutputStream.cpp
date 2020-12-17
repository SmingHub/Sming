/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * QuotedPrintableOutputStream.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "QuotedPrintableOutputStream.h"

size_t QuotedPrintableOutputStream::transform(const uint8_t* source, size_t sourceLength, uint8_t* target,
											  size_t targetLength)
{
	unsigned count = 0;
	for(unsigned i = 0; i < sourceLength; i++) {
		char byte = source[i];

		if((byte == ' ') || ((byte >= 33) && (byte <= 126) && (byte != '='))) {
			target[count++] = byte;
		} else {
			target[count++] = '=';
			target[count++] = hexchar(byte >> 4);
			target[count++] = hexchar(byte & 0x0F);
		}
	}

	return count;
}
