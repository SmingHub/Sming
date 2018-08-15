/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TemplateStream.h"

/* TemplateStream */

size_t TemplateStream::readMemoryBlock(char* data, size_t bufSize)
{
	debug_d("READ Template (%d)", _state);
	int available;

	if (_state == eTES_StartVar) {
		if (_templateData.contains(_varName)) {
			// Return variable value
			debug_d("StartVar %s", _varName.c_str());
			const String& value = _templateData[_varName];
			available = value.length();
			memcpy(data, value.c_str(), available);
			_stream->seek(_skipBlockSize);
			_varDataPos = 0;
			_state = eTES_SendingVar;
			return available;
		}
		else {
			debug_d("var %s not found", _varName.c_str());
			_state = eTES_Wait;
			auto len = _stream->readMemoryBlock(data, bufSize);
			return (len < _skipBlockSize) ? len : _skipBlockSize;
		}
	}
	else if (_state == eTES_SendingVar) {
		String* val = &_templateData[_varName];
		if (_varDataPos < val->length()) {
			debug_d("continue TRANSFER variable value (not completed)");
			available = val->length() - _varDataPos;
			memcpy(data, val->c_str() + _varDataPos, available);
			return available;
		}
		else {
			debug_d("continue to plain text");
			_state = eTES_Wait;
		}
	}

	unsigned len = _stream->readMemoryBlock(data, bufSize);
	const char* tpl = data;
	if (tpl && len > 0) {
		auto end = tpl + len;
		auto cur = (const char*)memchr(tpl, '{', len);
		auto lastFound = cur;
		while (cur) {
			lastFound = cur;
			const char* p = cur + 1;
			for (; p < end; p++) {
				if (isspace(*p))
					break; // Not a var name

				if (p - cur > TEMPLATE_MAX_VAR_NAME_LEN)
					break; // To long for var name

				if (*p == '{')
					break; // New start..

				if (*p != '}')
					continue;

				// name without { and }
				unsigned namelen = p - cur - 1;
				if (!_varName.setLength(namelen))
					return 0;	// Memory allocation failure
				memcpy(_varName.begin(), cur + 1, namelen);
				_skipBlockSize = p - cur + 1;
				_varWaitSize = cur - tpl;
				_state = _varWaitSize ? eTES_Found : eTES_StartVar;
				debug_d("found var: %s, at %u - %u, send size %u", _varName.c_str(), _varWaitSize + 1, p - tpl,
						_varWaitSize);

				// return only plain text from template without our variable
				return _varWaitSize;
			}
			cur = (const char*)memchr(p, '{', len - (p - tpl)); // continue searching..
		}

		if (lastFound && (unsigned)(lastFound - tpl) > (len - TEMPLATE_MAX_VAR_NAME_LEN)) {
			debug_d("trim end to %d from %d", lastFound - tpl, len);
			len = lastFound - tpl; // It can be a incomplete variable name. Don't split it!
		}
	}

	//	debug_d("plain template text pos: %d, len: %d", getPos(), len);
	return len;
}

bool TemplateStream::seek(int len)
{
	// Forward-only seeks
	if (len < 0)
		return false;
	//debug_d("SEEK: %d, (%d)", len, state);

	if (_state == eTES_Found) {
		//debug_d("SEEK before Var: %d, (%d)", len, varWaitSize);
		_varWaitSize -= len;
		if (_varWaitSize == 0)
			_state = eTES_StartVar;
	}
	else if (_state == eTES_SendingVar) {
		_varDataPos += len;
		return false; // not the end
	}

	return _stream->seek(len);
}
