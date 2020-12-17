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

String TemplateStream::evaluate(char*& expr)
{
	auto end = strchr(expr, '}');
	if(end == nullptr) {
		return nullptr;
	}

	*end = '\0';
	String s = getValue(expr);
	*end = '}';
	expr = end + 1;
	return s;
}

String TemplateStream::getValue(const char* name)
{
	String s = static_cast<const Variables&>(templateData)[name];
	if(!s && getValueCallback) {
		s = getValueCallback(name);
	}
	debug_d("TemplateStream: value '%s' %sfound: \"%s\"", name, s ? "" : "NOT ", s.c_str());
	return s;
}

uint16_t TemplateStream::readMemoryBlock(char* data, int bufSize)
{
	if(data == nullptr || bufSize <= 0) {
		return 0;
	}

	auto sendValue = [&]() {
		assert(value.length() != 0);
		auto len = std::min(size_t(bufSize), value.length() - valuePos);
		memcpy(data, value.c_str() + valuePos, len);
		sendingValue = true;
		return len;
	};

	if(sendingValue) {
		return sendValue();
	}

	const size_t tagDelimiterLength = 1 + doubleBraces;

	if(size_t(bufSize) <= TEMPLATE_MAX_VAR_NAME_LEN + (2 * tagDelimiterLength)) {
		return 0;
	}

	auto findStartTag = [this](char* buf) {
		auto p = doubleBraces ? strstr(buf, "{{") : strchr(buf, '{');
		return static_cast<char*>(p);
	};

	auto start = data;
	size_t datalen = stream->readMemoryBlock(data, bufSize - 1);
	if(datalen != 0) {
		data[datalen] = '\0'; // Terminate buffer to mitigate overflow risk
		auto tagStart = findStartTag(data);
		auto lastTagFound = tagStart;
		while(tagStart != nullptr) {
			lastTagFound = tagStart;

			char* curPos = tagStart + tagDelimiterLength;
			value = evaluate(curPos);
			if(doubleBraces) {
				// Double end brace isn't necessary, but if present skip it
				if(*curPos == '}') {
					++curPos;
				}
			}
			if(!value) {
				// Not handled, emit unchanged and continue searching
				tagStart = findStartTag(curPos);
				continue;
			}
			auto tagEnd = curPos;
			tagLength = tagEnd - tagStart;
			valueWaitSize = tagStart - start;

			if(valueWaitSize == 0 || !outputEnabled) {
				stream->seek(valueWaitSize + tagLength);
				valueWaitSize = 0;
				tagLength = 0;
				start = tagEnd;
				if(!outputEnabled || value.length() == 0) {
					value = nullptr;
					outputEnabled = enableNextState;
					tagStart = findStartTag(curPos);
					continue;
				}
			}

			if(outputEnabled && valueWaitSize == 0 && value.length() != 0) {
				valuePos = 0;
				return sendValue();
			}

			outputEnabled = enableNextState;

			// return only plain text from template without our variable
			memmove(data, start, valueWaitSize);
			return valueWaitSize;
		}

		if(lastTagFound != nullptr) {
			unsigned newlen = lastTagFound - data;
			if(newlen + TEMPLATE_MAX_VAR_NAME_LEN > datalen) {
				debug_d("TemplateStream: trim end to %u from %u", newlen, datalen);
				// It can be a incomplete variable name - don't split it
				// provided we're not at end of input stream
				if(datalen == size_t(bufSize)) {
					datalen = newlen;
				}
			}
		}
	}

	datalen -= (start - data);
	if(outputEnabled) {
		memmove(data, start, datalen);
	} else {
		stream->seek(datalen);
		datalen = 0;
	}

	return datalen;
}

int TemplateStream::seekFrom(int offset, SeekOrigin origin)
{
	if(origin == SeekOrigin::Start && offset == 0) {
		if(stream->seekFrom(0, SeekOrigin::Start) != 0) {
			return 0;
		}
		reset();
		streamPos = 0;
		return streamPos;
	}

	// Forward-only seeks
	if(origin != SeekOrigin::Current || offset < 0) {
		return -1;
	}

	if(sendingValue) {
		valuePos += offset;
		assert(valuePos <= value.length());
		if(valuePos >= value.length()) {
			sendingValue = false;
			value = nullptr;
		}
		streamPos += offset;
		return streamPos;
	}

	if(valueWaitSize != 0) {
		if(size_t(offset) > valueWaitSize) {
			debug_e("TemplateStream: offset > valueWaitSize");
			return -1;
		}
		valueWaitSize -= offset;
		if(valueWaitSize == 0) {
			stream->seek(tagLength);
			tagLength = 0;
			if(value.length() != 0) {
				valuePos = 0;
				sendingValue = true;
			}
		}
	}

	if(stream->seekFrom(offset, SeekOrigin::Current) < 0) {
		return -1;
	}

	streamPos += offset;
	return streamPos;
}
