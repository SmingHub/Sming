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
static int quotedPrintableTransformer(uint8_t* source, size_t sourceLength, uint8_t* target, size_t targetLength)
{
	const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	int count = 0;
	for(int i = 0; i < sourceLength; i++) {
		char byte = source[i];

		if((byte == ' ') || ((byte >= 33) && (byte <= 126) && (byte != '='))) {
			target[count++] = byte;
		} else {
			target[count++] = '=';
			target[count++] = hex[((byte >> 4) & 0x0F)];
			target[count++] = hex[(byte & 0x0F)];
		}
	}

	return count;
}

QuotedPrintableOutputStream::QuotedPrintableOutputStream(ReadWriteStream* stream, size_t resultSize /* = 512 */)
	: StreamTransformer(stream, nullptr, resultSize, resultSize / 2)

{
	transformCallback = quotedPrintableTransformer;
}
