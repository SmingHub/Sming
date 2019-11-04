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

uint16_t TemplateStream::readMemoryBlock(char* data, int bufSize)
{
	debug_d("TemplateStream::read(%d), state = %d", bufSize, state);

	if(!data || bufSize <= 0)
		return 0;

	if(state == eTES_StartVar) {
		int i = templateData.indexOf(varName);
		debug_d("StartVar '%s' %sfound", varName.c_str(), i < 0 ? "NOT " : "");
		if(i < 0) {
			state = eTES_Wait;
			return stream->readMemoryBlock(data, std::min(size_t(bufSize), skipBlockSize));
		}

		// Return variable value
		const String& value = templateData.valueAt(i);
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
		const String& val = templateData[varName];
		if(varDataPos < val.length()) {
			debug_d("continue TRANSFER variable value (not completed)");
			size_t available = val.length() - varDataPos;
			memcpy(data, val.c_str() + varDataPos, available);
			return available;
		} else {
			debug_d("continue to plain text");
			state = eTES_Wait;
		}
	}

	unsigned datalen = stream->readMemoryBlock(data, bufSize);
	if(datalen != 0) {
		auto end = data + datalen;
		auto cur = (const char*)memchr(data, '{', datalen);
		auto lastFound = cur;
		while(cur != nullptr) {
			lastFound = cur;
			const char* p = cur + 1;
			for(; p < end; p++) {
				if(isspace(*p))
					break; // Not a var name
				else if(p - cur > TEMPLATE_MAX_VAR_NAME_LEN)
					break; // To long for var name
				else if(*p == '{')
					break; // New start..

				if(*p == '}') {
					varName.setLength(p - cur - 1);
					memcpy(varName.begin(), cur + 1, varName.length()); // name without { and }
					skipBlockSize = p - cur + 1;
					varWaitSize = cur - data;
					state = varWaitSize ? eTES_Found : eTES_StartVar;
					debug_d("found var '%s' at %u - %u, send size %u", varName.c_str(), varWaitSize + 1, p - data,
							varWaitSize);

					// return only plain text from template without our variable
					return varWaitSize;
				}
			}

			// continue searching...
			cur = (const char*)memchr(p, '{', datalen - (p - data));
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
	if(len < 0)
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
