/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * LineBuffer.h - support for buffering/editing a line of text
 *
 * author mikee47 <mike@sillyhouse.net> Feb 2019
 *
 ****/

#include "LineBuffer.h"

char LineBufferBase::addChar(char c)
{
	if(c == '\n' || c == '\r') {
		return '\n';
	}

	if(c >= 0x20 && c < 0x7f && length < (size - 1)) {
		buffer[length++] = c;
		buffer[length] = '\0';
		return c;
	}

	return '\0';
}

bool LineBufferBase::backspace()
{
	if(length == 0) {
		return false;
	}
	--length;
	buffer[length] = '\0';
	return true;
}

bool LineBufferBase::startsWith(const char* text) const
{
	auto len = strlen(text);
	return memcmp(buffer, text, len) == 0;
}

bool LineBufferBase::contains(const char* text) const
{
	return strstr(buffer, text) != nullptr;
}
