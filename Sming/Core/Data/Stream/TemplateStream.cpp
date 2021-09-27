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
		expr += strlen(expr);
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
	debug_d("[TMPL] value '%s' %sfound: \"%s\"", name, s ? "" : "NOT ", s.c_str());
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

	if(valueWaitSize != 0) {
		debug_d("[TMPL] #1");
		size_t res = std::min(uint16_t(bufSize), valueWaitSize);
		return stream->readMemoryBlock(data, res);
	}

	const size_t tagDelimiterLength = 1 + doubleBraces;

	if(size_t(bufSize) <= TEMPLATE_MAX_VAR_NAME_LEN + (2 * tagDelimiterLength)) {
		debug_d("[TMPL] #2");
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
		while(tagStart != nullptr) {
			char* curPos = tagStart + tagDelimiterLength;
			value = evaluate(curPos);
			size_t tailpos = curPos - data;
			if(tailpos >= datalen) {
				debug_d("[TMPL #3]");
				// Incomplete variable name, end tag not found in buffer
				unsigned newlen = tagStart - data;
				if(newlen + TEMPLATE_MAX_VAR_NAME_LEN > datalen) {
					// Return what we have so far, unless we're at the end of the input stream
					if(datalen == size_t(bufSize - 1)) {
						debug_d("[TMPL #4]");
						datalen = newlen;
						break;
					}
				}
			}
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
			debug_e("[TMPL] offset > valueWaitSize");
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
