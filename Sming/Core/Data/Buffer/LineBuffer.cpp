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

LineBufferBase::Action LineBufferBase::process(Stream& input, ReadWriteStream& output)
{
	int c;
	while((c = input.read()) >= 0) {
		auto action = processKey(c, &output);
		if(action == Action::clear || action == Action::submit) {
			return action;
		}
	}
	return Action::none;
}

LineBufferBase::Action LineBufferBase::processKey(char key, ReadWriteStream* output)
{
	auto prevKey = previousKey;
	previousKey = key;

	switch(key) {
	case '\x1b': // ESC -> delete current commandLine
		clear();
		if(output) {
			output->println();
		}
		return Action::clear;

	case '\b':   // delete (backspace)
	case '\x7f': // xterm ctrl-?
		if(!backspace()) {
			return Action::none;
		}
		if(output) {
			output->print("\b \b");
		}
		return Action::backspace;

	case '\r':
	case '\n':
		// For "\r\n" or "\n\r" sequence ignore second key
		if(prevKey != key && (prevKey == '\r' || prevKey == '\n')) {
			previousKey = '\0';
			return Action::none;
		}
		if(output) {
			output->println();
		}
		return Action::submit;

	default:
		if(!addChar(key)) {
			return Action::none;
		}
		if(output) {
			output->print(key);
		}
		return Action::echo;
	}
}

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
