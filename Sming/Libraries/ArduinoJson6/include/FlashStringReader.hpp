// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License
//
// Sming FlashString reader mikee47 Nov 2019 <mike@sillyhouse.net>

#pragma once

namespace ARDUINOJSON_NAMESPACE
{
template <> struct Reader<FlashString, void> {
	explicit Reader(const FlashString& str) : str(str)
	{
	}

	int read()
	{
		if(index >= str.length()) {
			return -1;
		}
		unsigned char c = str[index];
		++index;
		return c;
	}

	size_t readBytes(char* buffer, size_t length)
	{
		auto count = str.read(index, buffer, length);
		index += count;
		return count;
	}

private:
	const FlashString& str;
	unsigned index = 0;
};

} // namespace ARDUINOJSON_NAMESPACE
