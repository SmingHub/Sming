/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TemplateStream.cpp
 *
 ****/

#include "TemplateStream.h"

String TemplateStream::getValue(const char* name)
{
	String s = static_cast<const Variables&>(templateData)[name];
	if(!s && getValueCallback) {
		s = getValueCallback(name);
	}
	debug_d("StartVar '%s' %sfound: \"%s\"", name, s ? "" : "NOT ", s.c_str());
	return s;
}

uint16_t TemplateStream::readMemoryBlock(char* data, int bufSize)
{
	debug_d("TemplateStream::read(%d), state = %d", bufSize, state);

	if(!data || bufSize <= 0)
		return 0;

	if(state == eTES_StartVar) {
		if(!value) {
			debug_d("var not found");
			state = eTES_Wait;
			return stream->readMemoryBlock(data, std::min(size_t(bufSize), skipBlockSize));
		}

		// Return variable value
		if(unsigned(bufSize) < value.length()) {
			debug_e("TemplateStream, buffer too small");
			return 0;
		}
		memcpy(data, value.c_str(), value.length());
		stream->seek(skipBlockSize);
		varDataPos = 0;
		state = eTES_SendingVar;
		return value.length();
	}

	if(state == eTES_SendingVar) {
		if(varDataPos < value.length()) {
			debug_d("continue TRANSFER variable value (not completed)");
			size_t available = value.length() - varDataPos;
			memcpy(data, value.c_str() + varDataPos, available);
			return available;
		} else {
			debug_d("continue to plain text");
			state = eTES_Wait;
		}
	}

	unsigned datalen = stream->readMemoryBlock(data, bufSize);
	if(datalen != 0) {
		auto end = data + datalen;
		auto cur = static_cast<char*>(memchr(data, '{', datalen));
		auto lastFound = cur;
		while(cur != nullptr) {
			lastFound = cur;
			char* p = cur + 1;
			for(; p < end; p++) {
				if(isspace(*p))
					break; // Not a var name
				else if(p - cur > TEMPLATE_MAX_VAR_NAME_LEN)
					break; // To long for var name
				else if(*p == '{')
					break; // New start..

				if(*p == '}') {
					*p = '\0';
					const char* varName = cur + 1;
					value = getValue(varName);
					skipBlockSize = p - cur + 1;
					varWaitSize = cur - data;
					state = varWaitSize ? eTES_Found : eTES_StartVar;
					debug_d("found var '%s' at %u - %u, send size %u", varName, varWaitSize + 1, p - data, varWaitSize);

					// return only plain text from template without our variable
					return varWaitSize;
				}
			}

			// continue searching...
			cur = static_cast<char*>(memchr(p, '{', datalen - (p - data)));
		}

		if(lastFound != nullptr) {
			unsigned newlen = lastFound - data;
			if(newlen + TEMPLATE_MAX_VAR_NAME_LEN > datalen) {
				debug_d("trim end to %u from %u", newlen, datalen);
				// It can be a incomplete variable name - don't split it
				datalen = newlen;
			}
		}
	}

	debug_d("plain template text pos: %d, len: %d", -1, datalen);
	return datalen;
}

bool TemplateStream::seek(int len)
{
	debug_d("TemplateStream::seek(%d), state = %d", len, state);

	// Forward-only seeks
	if(len <= 0)
		return false;

	if(state == eTES_Found) {
		//debug_d("SEEK before Var: %d, (%d)", len, varWaitSize);
		if(varWaitSize < (unsigned)len) {
			debug_e("len > varWaitSize");
			return false;
		}
		varWaitSize -= len;
		if(varWaitSize == 0)
			state = eTES_StartVar;
	} else if(state == eTES_SendingVar) {
		varDataPos += len;
		return false; // not the end
	}

	return stream->seek(len);
}
