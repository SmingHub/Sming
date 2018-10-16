/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 */

#include "HexString.h"

String makeHexString(const uint8_t* data, unsigned length, char separator)
{
	String result;

	unsigned charsPerByte = separator ? 3 : 2;
	if(!result.setLength(charsPerByte * length + 1))
		return nullptr;

	char* p = result.begin();
	for(unsigned i = 0; i < length; ++i) {
		if(i && separator)
			*p++ = separator;
		*p++ = hexchar(data[i] >> 4);
		*p++ = hexchar(data[i] & 0x0F);
	}
	*p = '\0';

	return result;
}
