/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "QuotedPrintableOutputStream.h"

/*
 * @brief Stream quoted-printable encoder
 * @param uint8_t* source - the incoming data
 * @param size_t sourceLength -length of the incoming data
 * @param uint8_t* target - the result data. The pointer must point to an already allocated memory
 * @param int* targetLength - the length of the allocated result data
 */
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
